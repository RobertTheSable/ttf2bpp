#ifndef RENDER_GLYPH_CONV_H
#define RENDER_GLYPH_CONV_H

#include "ttf2bpp/renderer.h"

namespace ttf2bpp {

GlyphData getGlyphData(unsigned long endoding, int width, int index);

}

#endif // RENDER_GLYPH_CONV_H
