#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <set>
#include "renderer.h"

namespace ttf2bpp {
struct Reserved {
    std::string label;
    bool isAlias = false;
    unsigned long code;
    int index;
    bool search = false;
    friend bool operator ==(const Reserved& lhs, const Reserved& rhs);
    friend bool operator <(const Reserved& lhs, const Reserved& rhs);
    friend bool operator ==(const unsigned long& lhs, const Reserved& rhs);
    friend bool operator ==(const Reserved& lhs, const unsigned long& rhs);
    friend bool operator ==(const std::string& lhs, const Reserved& rhs);
    friend bool operator ==(const Reserved& lhs, const std::string& rhs);
    friend bool operator !=(const Reserved& lhs, const Reserved& rhs);
};

struct Configuration {
    int spaceWidth;
    int maxCode = 0;
    std::set<Reserved> reservedGlyphs;
    // bool rightToLeft;
    std::vector<unsigned long> arrange(const std::vector<unsigned long>& input) const;
};

Configuration readConfiguration(const std::string& path);
void writeConfiguration(const std::string& path, Configuration config);
void writeFontData(const std::string& path, const std::vector<GlyphData>& data);
}

#endif // CONFIGURATION_H
