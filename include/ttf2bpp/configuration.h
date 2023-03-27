#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "ttf2bpp_exports.h"

#include <string>
#include <set>
#include <array>
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

struct TTF_BPP_EXPORT Configuration {
    enum class Colors {Background, Border, Text, Unused};
    int spaceWidth;
    int maxCode = 0;
    std::string extension = ".smc";
    std::set<Reserved> reservedGlyphs;
    std::array<Colors, 4> ordering = {Colors::Unused, Colors::Border, Colors::Text, Colors::Background};
    // bool rightToLeft;
    std::vector<unsigned long> arrange(const std::vector<unsigned long>& input) const;
    std::string getOutputPath(const std::string& in, const std::string& out) const;
};

TTF_BPP_EXPORT Configuration readConfiguration(const std::string& path);
TTF_BPP_EXPORT void writeConfiguration(const std::string& path, Configuration config);
TTF_BPP_EXPORT void writeFontData(const std::string& path, const std::vector<GlyphData>& data);
}

#endif // CONFIGURATION_H
