#include "render_glyph_conv.h"
#include "configuration/utf_conv.h"
#include <unicode/ustring.h>

ttf2bpp::GlyphData ttf2bpp::getGlyphData(unsigned long encoding, int width, int index)
{
    std::string utf8 = ttf2bpp::toUtf8(encoding);
    //boost::locale::conv::utf_to_utf<char>(std::basic_string<char32_t>{(char32_t)endoding});

    return GlyphData{
        .utf8Repr = utf8,
        .code = index,
        .length = width
    };
}
