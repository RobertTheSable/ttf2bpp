#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "ttf2bpp_exports.h"

#include <string>
#include <set>
#include <array>
#include <memory>
#include "renderer.h"

namespace ttf2bpp {
struct TTF_BPP_EXPORT Reserved {
    std::string label;
    bool isAlias = false;
    unsigned long code;
    int index;
    bool search = false;
    bool badCode = false;
    TTF_BPP_EXPORT friend bool operator ==(const Reserved& lhs, const Reserved& rhs);
    TTF_BPP_EXPORT friend bool operator <(const Reserved& lhs, const Reserved& rhs);
    TTF_BPP_EXPORT friend bool operator ==(const unsigned long& lhs, const Reserved& rhs);
    TTF_BPP_EXPORT friend bool operator ==(const Reserved& lhs, const unsigned long& rhs);
    TTF_BPP_EXPORT friend bool operator ==(const std::string& lhs, const Reserved& rhs);
    TTF_BPP_EXPORT friend bool operator ==(const Reserved& lhs, const std::string& rhs);
    TTF_BPP_EXPORT friend bool operator !=(const Reserved& lhs, const Reserved& rhs);
};

class TTF_BPP_EXPORT Configuration {
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
public:
    Configuration();
    Configuration(Configuration&&);
    ~Configuration();
    enum class Colors {Background, Border, Text, Unused};
    enum class WidthType {YAML, TASS64, CSV};
    enum class EncType {YAML, TASS64};
    struct options {
        int maxCode = 0;
        int glyphStart = 1;
        int baseline = 13, alphaThreshold = 112, borderPointSize = 40;
        int glyphWidth = GlyphDimention, renderWidth = GlyphDimention;
        int fontByteWidth = 1;
        EncType outEncoding = EncType::YAML;
        WidthType widthEncoding = WidthType::YAML;
        std::string encFileName;
        std::string extension = ".smc";
        bool encNameSet = false;
        std::set<Reserved> reservedGlyphs;
        std::vector<std::string> codeBlocks = {};
        std::array<Colors, 4> ordering = {Colors::Unused, Colors::Border, Colors::Text, Colors::Background};
    } params;
    options* operator->();
    const options* operator->() const;
    options& operator*();
    const options& operator*() const;
    // bool rightToLeft;
    Configuration& updateOutputParams(const std::string& fontFile, const std::string& encFile);
    Configuration&& changeOutputParams(const std::string& fontFile, const std::string& encFile);
    std::vector<GlyphInput> arrange(const std::vector<unsigned long>& input) const;
    std::string getOutputPath(const std::string& in, const std::string& out) const;
    int writeFontData(const std::vector<GlyphData> &data) const;
    Renderer getRenderer(ColorIndexes paletter, const std::string& file) const;
};

TTF_BPP_EXPORT Configuration readConfiguration(const std::string& path);
TTF_BPP_EXPORT void writeConfiguration(const std::string& path, const Configuration& config);
}

#endif // CONFIGURATION_H
