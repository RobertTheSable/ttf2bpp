#include "render_glyph_conv.h"

#include <boost/locale.hpp>

ttf2bpp::GlyphData ttf2bpp::getGlyphData(unsigned long endoding, int width, int index)
{
    std::string utf8 = boost::locale::conv::utf_to_utf<char>(std::basic_string<char32_t>{(char32_t)endoding});

    return GlyphData{
        .utf8Repr = utf8,
        .code = index,
        .length = width
    };
}
