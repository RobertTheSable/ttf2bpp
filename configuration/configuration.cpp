#include "config_impl.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <yaml-cpp/yaml.h>
#include <boost/locale.hpp>

namespace ttf2bpp {
    std::optional<unsigned long> fromUtf8(const std::string& utf8)
    {
         auto utf32Str = boost::locale::conv::utf_to_utf<char32_t>(utf8);
         if (utf32Str.length() != 1) {
             return std::nullopt;
         }
         return (unsigned long)utf32Str.front();
    }
    std::string toUtf8(unsigned long utf32)
    {
        return boost::locale::conv::utf_to_utf<char>(std::basic_string<char32_t>{(char32_t)utf32});
    }
}

namespace YAML {
    template <>
    struct convert<ttf2bpp::Reserved> {
        static bool decode(const Node& node, ttf2bpp::Reserved& rhs)
        {
            if (node["glyph"].IsDefined()) {
                if (auto val = ttf2bpp::fromUtf8(node["glyph"].as<std::string>()); (bool)val) {
                    rhs.code = *val;
                    rhs.isAlias = true;
                } else {
                    return false;
                }
            }
            rhs.index = node["code"].as<int>();
            return true;
        }
    };
    template <>
    struct convert<ttf2bpp::Configuration> {
        static bool decode(const Node& node, ttf2bpp::Configuration& rhs)
        {
            if (auto rGlyphNode = node["ReservedGlyphs"]; rGlyphNode.IsDefined() && rGlyphNode.IsMap()) {
                for (auto rGlyphNode: rGlyphNode) {
                    auto reservedGlyph = rGlyphNode.second.as<ttf2bpp::Reserved>();
                    reservedGlyph.label = rGlyphNode.first.as<std::string>();
                    if (!reservedGlyph.isAlias) {
                        if (auto val = ttf2bpp::fromUtf8(reservedGlyph.label); (bool)val) {
                            reservedGlyph.code = *val;
                        } else {
                            return false;
                        }
                    }
                    if (reservedGlyph.index > rhs.maxCode) {
                        rhs.maxCode = reservedGlyph.index;
                    }
                    rhs.reservedGlyphs.insert(reservedGlyph);
                }
            }
            if (auto spaceWidthNode = node["SpaceWidth"]; spaceWidthNode.IsDefined()) {
                if (!spaceWidthNode.IsScalar()) {
                    return false;
                }
            } else {
                rhs.spaceWidth = 3;
            }
            return true;
        }
        static Node encode(const ttf2bpp::Configuration& rhs) {
            Node n;
            n["SpaceWidth"] = rhs.spaceWidth;
            Node reservedGlyphNode;
            for(auto reservedGlyph: rhs.reservedGlyphs) {
                Node subNode;
                subNode["code"] = reservedGlyph.index;
                reservedGlyphNode[ttf2bpp::toUtf8(reservedGlyph.code)] = subNode;
            }
            return n;
        }
    };
    template <>
    struct convert<ttf2bpp::GlyphData> {
        static Node encode(const ttf2bpp::GlyphData& rhs) {
            auto n = Node();
            std::ostringstream fmt;
            fmt << "0x" << std::hex << rhs.code;
            n["code"] = fmt.str();
            n["length"] = rhs.length;
            return n;
        }
    };
}

namespace ttf2bpp {

bool operator ==(const Reserved& lhs, const Reserved& rhs)
{
    return lhs.code == rhs.code;
}
bool operator <(const Reserved& lhs, const Reserved& rhs)
{
    if (lhs.search || rhs.search) {
        return lhs.code < rhs.code;
    }
    return lhs.index < rhs.index;
}
bool operator ==(const unsigned long& lhs, const Reserved& rhs)
{
    return lhs == rhs.code;
}
bool operator ==(const Reserved& lhs, const unsigned long& rhs)
{
    return lhs.code == rhs;
}
bool operator ==(const std::string& lhs, const Reserved& rhs)
{
    return lhs == rhs.label;
}
bool operator ==(const Reserved& lhs, const std::string& rhs)
{
    return lhs.label == rhs;
}

bool operator !=(const Reserved& lhs, const Reserved& rhs)
{
    return ! (lhs == rhs);
}

Configuration readConfiguration(const std::string& path)
{
    if (std::filesystem::exists(std::filesystem::path(path))) {
        try {
            auto node = YAML::LoadFile(path);
            return node.as<Configuration>();
        } catch (YAML::Exception& e) {
            throw std::runtime_error(e.what());
        }
    }
    auto defConfiguration = Configuration{
        .spaceWidth = 3
    };
    return defConfiguration;

}
void writeConfiguration(const std::string& path, Configuration config)
{
    if (std::filesystem::exists(std::filesystem::path(path))) {
        return;
    }
    std::ofstream output(path);
    YAML::Node outnode;
    outnode = config;
    output << outnode;
    output.close();
}
void writeFontData(const std::string& path, const std::vector<GlyphData>& data)
{
    std::ofstream output(path);
    YAML::Node n;
    for (auto& gl: data) {
        n[gl.utf8Repr] = gl;
    }
    output << n;
    output.close();
}


configuration::configuration()=default;

std::vector<unsigned long> ttf2bpp::Configuration::arrange(const std::vector<unsigned long> &input) const
{
    std::vector<unsigned long> output(maxCode+1, 0);
    auto source = input.begin();
    int idx = 0;
    for (auto& r: reservedGlyphs) {
        while(idx < r.index) {
            if (reservedGlyphs.find(Reserved{.code = *source, .search = true}) == reservedGlyphs.end()) {
                output[idx] = *source;
                ++idx;
            }
            if (source+1 != input.end()) {
                ++source;
            }
        }
        output[idx] = r.code;
        ++idx;
    }
    ++source;
    while (source != input.end()) {
        output[idx] = *source;
        ++idx;
        ++source;
    }
    return output;
}

} // namespace ttf2bpp
