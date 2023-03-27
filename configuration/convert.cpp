#include "config_impl.h"

#include <yaml-cpp/yaml.h>
#include <boost/locale.hpp>
#include <boost/locale/localization_backend.hpp>

namespace ttf2bpp {
    std::optional<unsigned long> fromUtf8(const std::string& utf8)
    {
        auto locale = boost::locale::generator().generate("en_US.utf-8");
        auto normalized = boost::locale::normalize(
            boost::locale::normalize(utf8, boost::locale::norm_nfd, locale),
            boost::locale::norm_nfc,
            locale
        );
        auto utf32Str = boost::locale::conv::utf_to_utf<char32_t>(normalized);
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
    bool convert<ttf2bpp::Reserved>::decode(const Node& node, ttf2bpp::Reserved& rhs)
    {
        if (!node.IsMap()) {
            return false;
        }
        if (node["glyph"].IsDefined()) {
            if (auto val = ttf2bpp::fromUtf8(node["glyph"].as<std::string>()); (bool)val) {
                rhs.code = *val;
                rhs.isAlias = true;
            } else {
                throw YAML::Exception(node.Mark(), "Glyphs must correspond to a single UTF32 code point.");
            }
        }
        rhs.index = node["code"].as<int>();
        return true;
    }

    bool convert<ttf2bpp::Configuration::Colors>::decode(const Node& node, Color& rhs)
    {
        if (!node.IsScalar()) {
            return false;
        }
        if (auto val = node.Scalar(); val == BG) {
            rhs = Color::Background;
        } else if (val == Text) {
            rhs = Color::Text;
        } else if (val == Border) {
            rhs = Color::Border;
        } else {
            rhs = Color::Unused;
        }
        return true;
    }

    Node convert<ttf2bpp::Configuration::Colors>::encode(const Color& rhs)
    {
        Node n;
        switch (rhs) {
        case Color::Background:
            n = "Background";
            break;
        case Color::Border:
            n = "Border";
            break;
        case Color::Text:
            n = "Text";
            break;
        default:
            n = "Unused";
            break;
        }
        return n;
    }

    bool convert<ttf2bpp::Configuration>::decode(const Node& node, ttf2bpp::Configuration& rhs)
    {
        if (auto rGlyphNode = node["ReservedGlyphs"]; rGlyphNode.IsDefined() && rGlyphNode.IsMap()) {
            for (auto it = rGlyphNode.begin(); it != rGlyphNode.end(); ++it) {
                auto reservedGlyph = it->second.as<ttf2bpp::Reserved>();
                reservedGlyph.label = it->first.as<std::string>();
                if (!reservedGlyph.isAlias) {
                    if (auto val = ttf2bpp::fromUtf8(reservedGlyph.label); (bool)val) {
                        reservedGlyph.code = *val;
                    } else {
                        throw YAML::Exception(node.Mark(), "Glyphs must correspond to a single UTF32 code point.");
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
        if (auto colorNode = node["Palette"]; colorNode.IsDefined()) {
            using Color = ttf2bpp::Configuration::Colors;
            if (!colorNode.IsSequence()) {
                return false;
            }
            auto data = colorNode.as<std::vector<Color>>();
            if (data.size() != 4) {
                return false;
            }
            std::copy(data.begin(), data.end(), rhs.ordering.begin());
        }
        if (auto extNode = node["Extension"]; extNode.IsDefined()) {
            if (!extNode.IsScalar()) {
                return false;
            }
            rhs.extension = extNode.as<std::string>();
        }
        return true;
    }
    Node convert<ttf2bpp::Configuration>::encode(const ttf2bpp::Configuration& rhs) {
        Node n;
        n["SpaceWidth"] = rhs.spaceWidth;
        Node reservedGlyphNode;
        for(auto reservedGlyph: rhs.reservedGlyphs) {
            Node subNode;
            subNode["code"] = reservedGlyph.index;
            reservedGlyphNode[ttf2bpp::toUtf8(reservedGlyph.code)] = subNode;
        }
        n["Palette"] = rhs.ordering;
        n["Extension"] = rhs.extension;
        return n;
    }
    Node convert<ttf2bpp::GlyphData>::encode(const ttf2bpp::GlyphData& rhs) {
        auto n = Node();
        std::ostringstream fmt;
        fmt << "0x" << std::hex << rhs.code;
        n["code"] = fmt.str();
        n["length"] = rhs.length;
        return n;
    }
}
