#include "config_impl.h"

#include <yaml-cpp/yaml.h>

namespace YAML {
    bool isNonDefault(const Node& node) {
        return node.IsDefined() && !node.IsNull();
    }
    bool convert<ttf2bpp::Reserved>::decode(const Node& node, ttf2bpp::Reserved& rhs)
    {
        if (!node.IsMap()) {
            return false;
        }
        if (auto gNode = node["glyph"]; isNonDefault(gNode)) {
            if (auto val = ttf2bpp::fromUtf8(gNode.as<std::string>()); (bool)val) {
                rhs.code = *val;
                rhs.isAlias = true;
            } else {
                throw YAML::Exception(node.Mark(), "Glyphs must correspond to a single UTF32 code point.");
            }
        }
        if (auto val= node["code"].as<int>(); val < 0) {
            throw YAML::Exception(node.Mark(), "Reserved glyphs must have a code value of at least 0.");
        } else {
            rhs.index = val;
        }
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
    constexpr const char* Skipped = "SkippedGlyphs";
    constexpr const char* Baseline = "Baseline";
    constexpr const char* AlphaThresh = "AlphaThreshold";
    constexpr const char* PT_Size = "BorderPointSize";
    constexpr const char* GlyphWidth = "GlyphWidth";
    constexpr const char* RenderWidth = "RenderWidth";
    constexpr const char* GlyphIndexStart = "GlyphIndexStart";
    constexpr const char* CodeBlocks = "CodeBlocks";
    constexpr const char* FontExt = "FontExtension";
    constexpr const char* EncFileName = "EncodingFile";
    constexpr const char* EncFileSub = "Filename";
    constexpr const char* EncSection = "Encoding";
    constexpr const char* EncExt = "Glyphs";
    constexpr const char* WidthExt = "Width";
    constexpr const char* ByteWidth = "ByteWidth";
    bool convert<ttf2bpp::Configuration>::decode(const Node& node, ttf2bpp::Configuration& rhs)
    {
        if (auto rGlyphNode = node[Reserved]; isNonDefault(rGlyphNode) && rGlyphNode.IsMap()) {
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
                if (reservedGlyph.index > rhs->maxCode) {
                    rhs->maxCode = reservedGlyph.index;
                }
                rhs->reservedGlyphs.insert(reservedGlyph);
            }
        }
        if (auto sGlyphNode = node[Skipped]; isNonDefault(sGlyphNode) && sGlyphNode.IsSequence()) {
            for (auto skippedEntry: sGlyphNode) {
                auto str = skippedEntry.as<std::string>();
                if (auto val = ttf2bpp::fromUtf8(str); (bool)val) {
                    rhs->skippedGlyphs.insert(str);
                } else {
                    throw YAML::Exception(node.Mark(), "Glyphs must correspond to a single UTF32 code point.");
                }
            }
        }
        if (auto colorNode = node[Palette]; isNonDefault(colorNode)) {
            using Color = ttf2bpp::Configuration::Colors;
            if (!colorNode.IsSequence()) {
                return false;
            }
            auto data = colorNode.as<std::vector<Color>>();
            if (data.size() != 4) {
                return false;
            }
            std::copy(data.begin(), data.end(), rhs->ordering.begin());
        }
        if (auto extNode = node[FontExt]; isNonDefault(extNode)) {
            if (!extNode.IsScalar()) {
                return false;
            }
            rhs->extension = extNode.as<std::string>();
        }
        if (auto bsl = node[Baseline] ; isNonDefault(bsl)) {
            if (!bsl.IsScalar()) {
                return false;
            }
            rhs->baseline = bsl.as<int>();
        }
        if (auto alpha = node[AlphaThresh] ; isNonDefault(alpha)) {
            if (!alpha.IsScalar()) {
                return false;
            }
            rhs->alphaThreshold = alpha.as<int>();
        }
        if (auto ptSize = node[PT_Size] ; isNonDefault(ptSize)) {
            if (!ptSize.IsScalar()) {
                return false;
            }
            rhs->borderPointSize = ptSize.as<int>();
        }

        if (auto glyphSize = node[GlyphWidth] ; isNonDefault(glyphSize)) {
            if (!glyphSize.IsScalar()) {
                return false;
            }
            if (auto val = glyphSize.as<int>(); val != 8 && val != 16) {
                 throw YAML::Exception(node.Mark(), "Glyphs width must be 8 or 16.");
            } else {
                rhs->glyphWidth = val;
            }
        } else {
            rhs->glyphWidth = ttf2bpp::GlyphDimention;
        }
        if (auto renderSize = node[RenderWidth] ; isNonDefault(renderSize)) {
            if (!renderSize.IsScalar()) {
                return false;
            }
            rhs->renderWidth = renderSize.as<int>();
        } else {
            rhs->renderWidth = rhs->glyphWidth;
        }

        if (auto gls= node[GlyphIndexStart]; isNonDefault(gls)) {
            if (!gls.IsScalar()) {
                return false;
            }
            if (auto val = gls.as<int>(); val < 0) {
                return false;
            } else {
                rhs->glyphStart = val;
            }
        }

        bool filenameset = false;
        if (auto encSection = node[EncSection]; isNonDefault(encSection)) {
            if (!encSection.IsMap()) {
                return false;
            }
            auto eEnc = encSection[EncExt];
            bool yamlSet = false;
            if (isNonDefault(eEnc)) {
                rhs->outEncoding = eEnc.as<ttf2bpp::Configuration::EncType>();
                yamlSet = rhs->outEncoding == ttf2bpp::Configuration::EncType::YAML;
            } else {
                rhs->outEncoding = ttf2bpp::Configuration::EncType::YAML;
            }

            if (auto wEnc = encSection[WidthExt]; isNonDefault(wEnc) && !yamlSet) {
                rhs->widthEncoding = wEnc.as<ttf2bpp::Configuration::WidthType>();
            } else if (yamlSet || !isNonDefault(eEnc)) {
                rhs->widthEncoding = ttf2bpp::Configuration::WidthType::YAML;
            } else {
                rhs->widthEncoding = eEnc.as<ttf2bpp::Configuration::WidthType>();
            }
            if (auto fNode = encSection[EncFileSub]; isNonDefault(fNode)) {
                bool filenameset = true;
                if (!fNode.IsScalar()) {
                    return false;
                }
                rhs->encFileName = fNode.as<std::string>();
            }
            if (auto bwNode = encSection[ByteWidth]; isNonDefault(bwNode)) {
                if (!bwNode.IsScalar()) {
                    return false;
                }
                rhs->fontByteWidth = bwNode.as<int>();
            }
        }

        if (auto encName = node[EncFileName]; !filenameset && isNonDefault(encName)) {
            if (!encName.IsScalar()) {
                return false;
            }
            rhs->encFileName = encName.as<std::string>();
        }

        if (auto uBlk = node[CodeBlocks]; isNonDefault(uBlk)) {
            if (!uBlk.IsSequence()) {
                return false;
            }
            for (Node itr: uBlk) {
                rhs->codeBlocks.push_back(itr.as<std::string>());
            }
        }


        return true;
    }
    Node convert<ttf2bpp::Configuration>::encode(const ttf2bpp::Configuration& rhs) {
        Node n;

        if (!rhs->reservedGlyphs.empty()) {
            Node reservedGlyphNode;
            for(auto reservedGlyph: rhs->reservedGlyphs) {
                Node subNode;
                subNode["code"] = reservedGlyph.index;
                reservedGlyphNode[ttf2bpp::toUtf8(reservedGlyph.code)] = subNode;
            }
            n[Reserved] = reservedGlyphNode;
        }
        if (!rhs->skippedGlyphs.empty()) {
            Node skippedGlyphNodes;
            for(auto skippedGlyph: rhs->skippedGlyphs) {
                skippedGlyphNodes.push_back(skippedGlyph);
            }
            n[Skipped] = skippedGlyphNodes;
        }

        n[Palette] = rhs->ordering;
        n[FontExt] = rhs->extension;
        n[Baseline] = rhs->baseline;
        n[AlphaThresh] = rhs->alphaThreshold;
        n[PT_Size] = rhs->borderPointSize;
        if (rhs->glyphWidth != ttf2bpp::GlyphDimention) {
            n[GlyphWidth] = rhs->glyphWidth;
        }
        if (rhs->glyphWidth != rhs->renderWidth) {
            n[RenderWidth] = rhs->renderWidth;
        }
        Node encNode;
        encNode[EncExt] = rhs->outEncoding;
        encNode[WidthExt] = rhs->widthEncoding;
        encNode[ByteWidth] = rhs->fontByteWidth;
        if (rhs->encNameSet) {
            encNode[EncFileSub] = rhs->encFileName;
        }
        n[EncSection] = encNode;
        n[CodeBlocks] = rhs->codeBlocks;

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
