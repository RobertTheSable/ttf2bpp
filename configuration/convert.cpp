#include "config_impl.h"

#include <yaml-cpp/yaml.h>

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

    using Color = ttf2bpp::Configuration::Colors;
    static constexpr const char* BG = "Background";
    static constexpr const char* Text = "Text";
    static constexpr const char* Border = "Border";
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

    constexpr const char* Palette = "Palette";
    constexpr const char* Reserved = "ReservedGlyphs";
    constexpr const char* Baseline = "Baseline";
    constexpr const char* FontExt = "FontExtension";
    constexpr const char* EncFileName = "EncodingFile";
    //constexpr const char* EncExt = "EncodingType";
    constexpr const char* AlphaThresh = "AlphaThreshold";
    constexpr const char* PT_Size = "BorderPointSize";
    constexpr const char* GlyphWidth = "GlyphWidth";
    constexpr const char* RenderWidth = "RenderWidth";
    bool convert<ttf2bpp::Configuration>::decode(const Node& node, ttf2bpp::Configuration& rhs)
    {
        if (auto rGlyphNode = node[Reserved]; rGlyphNode.IsDefined() && rGlyphNode.IsMap()) {
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
        if (auto colorNode = node[Palette]; colorNode.IsDefined()) {
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
        if (auto encName = node[EncFileName]; encName.IsDefined()) {
            if (!encName.IsScalar()) {
                return false;
            }
            rhs.encFileName = encName.as<std::string>();
        }
        if (auto extNode = node[FontExt]; extNode.IsDefined()) {
            if (!extNode.IsScalar()) {
                return false;
            }
            rhs.extension = extNode.as<std::string>();
        }
        if (auto bsl = node[Baseline] ; bsl.IsDefined()) {
            if (!bsl.IsScalar()) {
                return false;
            }
            rhs.baseline = bsl.as<int>();
        }
        if (auto alpha = node[AlphaThresh] ; alpha.IsDefined()) {
            if (!alpha.IsScalar()) {
                return false;
            }
            rhs.alphaThreshold = alpha.as<int>();
        }
        if (auto ptSize = node[PT_Size] ; ptSize.IsDefined()) {
            if (!ptSize.IsScalar()) {
                return false;
            }
            rhs.borderPointSize = ptSize.as<int>();
        }

        if (auto glyphSize = node[GlyphWidth] ; glyphSize.IsDefined()) {
            if (!glyphSize.IsScalar()) {
                return false;
            }
            if (auto val = glyphSize.as<int>(); val != 8 && val != 16) {
                 throw YAML::Exception(node.Mark(), "Glyphs width must be 8 or 16.");
            } else {
                rhs.glyphWidth = val;
            }
        } else {
            rhs.glyphWidth = ttf2bpp::GlyphDimention;
        }
        if (auto renderSize = node[RenderWidth] ; renderSize.IsDefined()) {
            if (!renderSize.IsScalar()) {
                return false;
            }
            rhs.renderWidth = renderSize.as<int>();
        } else {
            rhs.renderWidth = rhs.glyphWidth;
        }
        return true;
    }
    Node convert<ttf2bpp::Configuration>::encode(const ttf2bpp::Configuration& rhs) {
        Node n;

        if (!rhs.reservedGlyphs.empty()) {
            Node reservedGlyphNode;
            for(auto reservedGlyph: rhs.reservedGlyphs) {
                Node subNode;
                subNode["code"] = reservedGlyph.index;
                reservedGlyphNode[ttf2bpp::toUtf8(reservedGlyph.code)] = subNode;
            }
            n[Reserved] = reservedGlyphNode;
        }

        n[Palette] = rhs.ordering;
        n[FontExt] = rhs.extension;
        n[Baseline] = rhs.baseline;
        n[AlphaThresh] = rhs.alphaThreshold;
        n[PT_Size] = rhs.borderPointSize;
        if (rhs.encNameSet) {
            n[EncFileName] = rhs.encFileName;
        }
        if (rhs.glyphWidth != ttf2bpp::GlyphDimention) {
            n[GlyphWidth] = rhs.glyphWidth;
        }
        if (rhs.glyphWidth != rhs.renderWidth) {
            n[RenderWidth] = rhs.renderWidth;
        }

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
