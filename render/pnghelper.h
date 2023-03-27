#ifndef TTF2BPP_PNGHELPER_H
#define TTF2BPP_PNGHELPER_H

#include <span>
#include <optional>
#include <cstdint>
#include <png++/png.hpp>

namespace ttf2bpp {

bool equals(const png::rgba_pixel& lhs, std::optional<png::rgba_pixel> rhs);
void composite(png::image<png::rgba_pixel>& low, png::image<png::rgba_pixel>& top, int xOff, int yOff, std::optional<png::rgba_pixel> bgcolor = std::nullopt);
png::image<png::rgba_pixel> buildPng(std::span<uint8_t> pixels, unsigned int w, unsigned int h);
png::image<png::rgba_pixel> buildPng(png::rgba_pixel color, unsigned int w, unsigned int h);
png::image<png::rgba_pixel> buildPng(png::image<png::rgba_pixel> base, int xPos, int yPos, int width, int height);

} // namespace ttf2bpp

#endif // TTF2BPP_PNGHELPER_H
