#include "pnghelper.h"

namespace ttf2bpp {

bool equals(const png::rgba_pixel& lhs, std::optional<png::rgba_pixel> rhs)
{
    if (!rhs) {
        return false;
    }
    return (lhs.alpha == rhs->alpha) &&
            (lhs.red == rhs->red) &&
            (lhs.blue == rhs->blue) &&
            (lhs.green == rhs->green);
}

void composite(png::image<png::rgba_pixel>& low, png::image<png::rgba_pixel>& top, int xOff, int yOff, std::optional<png::rgba_pixel> bgcolor)
{
    for (auto y = 0; y < top.get_height(); ++y) {
        for (auto x = 0; x < top.get_width(); ++x) {
            if (auto px = top.get_pixel(x, y); px.alpha != 0 && !equals(px, bgcolor) ) {
                int destX = x+xOff;
                int destY = y+yOff;
                low.set_pixel(destX, destY, px);
            }
        }
    }
}

png::image<png::rgba_pixel> buildPng(std::span<uint8_t> pixels, unsigned int w, unsigned int h)
{
    png::image<png::rgba_pixel> pgimg(w, h);
    auto pxItr = pixels.begin();
    for (auto y = 0; y < pgimg.get_height(); ++y) {
        for (auto x = 0; x < pgimg.get_width(); ++x) {
            auto red = *pxItr++;
            auto blue = *pxItr++;;
            auto green = *pxItr++;
            auto alpha = *pxItr++;

            pgimg.set_pixel(x, y, png::rgba_pixel{red, blue, green, alpha});
        }
    }
    return pgimg;
}

png::image<png::rgba_pixel> buildPng(png::rgba_pixel color, unsigned int w, unsigned int h)
{
    png::image<png::rgba_pixel> pgimg(w, h);
    for (auto y = 0; y < pgimg.get_height(); ++y) {
        for (auto x = 0; x < pgimg.get_width(); ++x) {
            pgimg.set_pixel(x, y, color);
        }
    }
    return pgimg;
}

png::image<png::rgba_pixel> buildPng(png::image<png::rgba_pixel> base, int xPos, int yPos, int width, int height)
{
    png::image<png::rgba_pixel> pgimg(width, height);
    for (auto y = 0; y < pgimg.get_height(); ++y) {
        for (auto x = 0; x < pgimg.get_width(); ++x) {
            pgimg.set_pixel(x, y, base.get_pixel(x + xPos, y + yPos));
        }
    }
    return pgimg;
}

} // namespace ttf2bpp
