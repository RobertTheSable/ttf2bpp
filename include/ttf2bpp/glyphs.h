#ifndef GLYPHS_H
#define GLYPHS_H

#include "ttf2bpp_exports.h"

#include <vector>
#include <string>
namespace ttf2bpp {
    TTF_BPP_EXPORT std::vector<unsigned long> readGlyphsFromPaintext(const std::string& filepath);
}


#endif // GLYPHS_H
