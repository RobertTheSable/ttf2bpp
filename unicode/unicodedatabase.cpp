#include "ttf2bpp/unicodedatabase.h"
#include <libxml/SAX.h>
#include <libxml/parser.h>
#include <cstring>
#include <algorithm>
#include <optional>
#include <stack>

namespace ttf2bpp {

namespace {
bool inGroup = false;
std::stack<std::string> tags;
std::optional<UnicodeDatabase> ds = std::nullopt;
std::string currentCategory;
std::string currentBlock;

void OnStartElementNs(
    void *ctx,
    const xmlChar *localname,
    const xmlChar *prefix,
    const xmlChar *URI,
    int nb_namespaces,
    const xmlChar **namespaces,
    int nb_attributes,
    int nb_defaulted,
    const xmlChar **attributes
) {
    if (nb_attributes == 0) {
        return;
    }
    std::string tagName{(char*)localname};
    if (tagName == "group") {
        std::string category, block;
        auto attrPtr = attributes;
        int left = nb_attributes;
        bool done = false;
        while (left >0) {
            if (*attrPtr != nullptr) {
                std::string name{(char*)*attrPtr};
                if (name.find_first_of("\"") == std::string::npos) {
                    --left;
                }
                if (name == "blk") {
                    do {
                        ++attrPtr;
                    } while (*attrPtr == nullptr);
                    block = std::string((char*)*attrPtr, (char*)*(attrPtr+1));
                } else if (name == "gc") {
                    do {
                        ++attrPtr;
                    } while (*attrPtr == nullptr);
                    category = std::string((char*)*attrPtr, (char*)*(attrPtr+1));
                }
            }
            if (category != "" && block != "") {
                break;
            }
            ++attrPtr;
        }
        if (left <= 0) {
            // error
        }
        currentBlock = block;
        auto groupCategory = category.front();
        if (ds->block == block && (groupCategory == 'L' || groupCategory == 'N' || groupCategory == 'M')) {
            inGroup = true;
            currentCategory = groupCategory;
        }
    } else if (
        tagName == "char" &&
        !tags.empty() &&
        tags.top() == "group" &&
        currentBlock == ds->block
    ) {
        std::string code = "", category = currentCategory;
        auto attrPtr = attributes;
        int left = nb_attributes;
        bool done = false;
        while (left >0) {
            if (*attrPtr != nullptr) {
                std::string name{(char*)*attrPtr};
                if (name.find_first_of("\"") == std::string::npos) {
                    --left;
                }
                if (name == "cp") {
                    do {
                        ++attrPtr;
                    } while (*attrPtr == nullptr);
                    code = std::string((char*)*attrPtr, (char*)*(attrPtr+1));
                } else if (name == "gc") {
                    do {
                        ++attrPtr;
                    } while (*attrPtr == nullptr);
                    category = std::string((char*)*attrPtr, (char*)*(attrPtr+1));
                }
            }
            ++attrPtr;
        }
        auto groupCategory = category.front();
        if (code != "" && (groupCategory == 'L' || groupCategory == 'N' || groupCategory == 'P' || category == "Zs")) {
            auto cp = std::stoul(code, nullptr, 16);
            ds->codes.push_back(cp);
        }
    }
    tags.push(tagName);
}

void OnEndElementNs(
    void* ctx,
    const xmlChar* localname,
    const xmlChar* prefix,
    const xmlChar* URI
) {
//    auto testDb = *ds;
    std::string currentTag = "";
    if (!tags.empty()) {
        currentTag = tags.top();
        tags.pop();
    }
    if (!tags.empty() && currentTag != "group") {
//        bool ig = inGroup;
//        std::string top = tags.top();
        if ((tags.top() == "group" || currentTag == "name-alias") && inGroup) {
            inGroup &= true;
        } else {
            inGroup = false;
        }
    } else {
        inGroup = false;
    }
}

void OnCharacters(void *ctx, const xmlChar *ch, int len) {
    int test = 1;
}

void attributeDeclSAXFunc (void * ctx,
    const xmlChar * elem,
    const xmlChar * fullname,
    int type,
    int def,
    const xmlChar * defaultValue,
    xmlEnumerationPtr tree) {

}

} // anonymous


UnicodeDatabase UnicodeDatabase::getUnicodeDatabase(const std::string &file, const std::string &section, const std::string &category)
{
    if (ds) {
        return *ds;
    }
    ds = UnicodeDatabase{section, category};
    xmlSAXHandler SAXHander;
    memset (&SAXHander, 0, sizeof (SAXHander));
    SAXHander.initialized = XML_SAX2_MAGIC;
    SAXHander.startElementNs = &OnStartElementNs;
    SAXHander.endElementNs = &OnEndElementNs;
    SAXHander.characters = &OnCharacters;
    xmlFreeDoc(xmlSAXParseFile(&SAXHander, file.c_str(), 0));
    xmlCleanupParser();
    return *ds;
}

} // namespace ttf2bpp
