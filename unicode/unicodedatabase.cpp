#include "ttf2bpp/unicodedatabase.h"
#include <expat.h>
#include <cstring>
#include <algorithm>
#include <optional>
#include <stack>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

namespace ttf2bpp {

namespace {
struct state {
    bool inGroup = false;
    std::stack<std::string> tags;
    UnicodeDatabase ds;
    std::string currentCategory;
    std::string currentBlock;
    std::set<std::string> blocks;
};


void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    auto m_state = reinterpret_cast<state*>(userData);
    std::string tagName{name};
    m_state->tags.push(tagName);
    if (*atts == nullptr) {
        return;
    }
    if (tagName == "group") {
        std::string category, block;
        auto attrPtr = atts;
        while (*attrPtr != nullptr) {
            std::string name{(char*)*attrPtr};
            ++attrPtr;
            std::string value{(char*)*attrPtr};
            if (name == "blk") {
                block = value;
            } else if (name == "gc") {
                category = value;
            }
            if (category != "" && block != "") {
                break;
            }
            ++attrPtr;
        }
        m_state->currentBlock = block;
        auto groupCategory = category.front();
        if (m_state->blocks.contains(block) && (groupCategory == 'L' || groupCategory == 'N' || groupCategory == 'M')) {
            m_state->inGroup = true;
            m_state->currentCategory = groupCategory;
        }
    } else if (
        tagName == "char"
    ) {
        if (!m_state->tags.empty() &&
            m_state->inGroup
        ) {
            std::string code = "", category = m_state->currentCategory;
            auto attrPtr = atts;
            while (*attrPtr != nullptr) {
                std::string name{(char*)*attrPtr};
                ++attrPtr;
                std::string value{(char*)*attrPtr};
                if (name == "cp") {
                    code = value;
                } else if (name == "gc") {
                    category = value;
                }
                ++attrPtr;
            }
            auto groupCategory = category.front();
            if (code != "" && (groupCategory == 'L' || groupCategory == 'N' || groupCategory == 'P' || category == "Zs")) {
                auto cp = std::stoul(code, nullptr, 16);
                m_state->ds.codes.push_back(cp);
            }
        }
    }
    //userData = reinterpret_cast<void*>(m_state);
}

void XMLCALL
endElement(void *userData, const XML_Char *name) {
    auto m_state = reinterpret_cast<state*>(userData);
    std::string tagName{name};

    if (!m_state->tags.empty() && m_state->tags.top() == tagName) {
        m_state->tags.pop();
    }

    if (tagName == "group") {
        m_state->inGroup = false;
    }
}

} // anonymous


UnicodeDatabase UnicodeDatabase::getUnicodeDatabase(const std::string &file, std::vector<std::string>&& blocks)
{
    std::ifstream input(file);
    if (!input) {
        throw std::runtime_error("Unicode database file not found.");
    }
    auto m_state = std::make_unique<state>();
    for (auto block: blocks) {
        m_state->blocks.insert(block);
    }

    XML_Parser parser = XML_ParserCreate("utf-8");
    XML_SetElementHandler(parser, startElement, endElement);
    int bytes_read =1;
    XML_SetUserData(parser, reinterpret_cast<void*>(m_state.get()));
    while (!input.eof()) {
        std::string line;
        if (!std::getline(input, line)) {
            throw std::runtime_error(std::string{"Couldn't read database file after."} + line);
        }
        line += '\n';
        auto buffer = XML_GetBuffer(parser, line.length());
        std::copy(line.begin(), line.end(), (char*)buffer);
        bytes_read = line.length();
        if (auto status = XML_ParseBuffer(parser, bytes_read, bytes_read == 0); !status) {
            auto code = XML_GetErrorCode(parser);
            std::cerr << "Got status: " << (int)code << '\n';
            throw std::runtime_error("Error parsing the database file.");
        }
    }

    XML_ParserFree(parser);
    return m_state->ds;
}

} // namespace ttf2bpp
