#include "render_glyph_conv.h"
#include "configuration/utf_conv.h"

ttf2bpp::GlyphData ttf2bpp::getGlyphData(unsigned long encoding, int width, int index)
{
    std::string utf8 = ttf2bpp::toUtf8(encoding);

    return GlyphData{
        .utf8Repr = utf8,
        .code = index,
        .length = width
    };
}
