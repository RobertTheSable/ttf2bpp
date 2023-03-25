#include <iostream>
#include <freetype/freetype.h>
#include <Magick++.h>
#include FT_STROKER_H
#include <optional>
#include <span>

constexpr const unsigned int QuantumMax = (1 << MAGICKCORE_QUANTUM_DEPTH) - 1;
constexpr const unsigned int QuantumAlphaTransparent = (MAGICKCORE_HDRI_ENABLE == 0) ? QuantumMax : 0;
constexpr const unsigned int GlyphDimention = 16;

namespace  {
    FT_Library  library;
    FT_Face     face;
    FT_Stroker  stroker;
    int         baseline = 13;
    int         alphaThreshold = 112;
    int         borderPointSize = 40;
}

bool RenderGlyphSection(int top, FT_Bitmap bitmap, std::span<unsigned char> pixels, unsigned char red, unsigned char green, unsigned char blue, bool keepOpacity, int left = 0)
{
    for (int y = 0; y < bitmap.rows; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            int realy = y + top;
            int realx = x + left;
            if (realy < 0 || realy >= GlyphDimention) {
                continue;
            }
            int target = realx + (realy * GlyphDimention);
            int source = x + (y * bitmap.width);

            auto px = pixels.begin()+(target*4);
            auto value = bitmap.buffer[source];
            unsigned char opacity;
            if (keepOpacity) {
                opacity = (bitmap.buffer[source] >= alphaThreshold) ? 255 : 0;
            } else {
                opacity = (bitmap.buffer[source] == 0) ? 0 : 255;
            }

            for (auto channel : {red, green, blue, opacity}) {
                *px = channel;
                ++px;
            }
        }
    }
    return true;
}

std::optional<Magick::Image> DrawGlyph(char c)
{
    auto glyph_index = FT_Get_Char_Index(face, c);
    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
        std::cerr << "Glyph not loaded.";
        return std::nullopt;
    }
    auto metrics = face->glyph->metrics;

    FT_Glyph glyph;
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true)) {
        std::cerr << "Glyph not rendered.";
        return std::nullopt;
    }
    auto bmpGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    int top = bmpGlyph->top;
    int borderLeft = bmpGlyph->left;
    std::vector<unsigned char> pixels(GlyphDimention * GlyphDimention * 4, 0);
    RenderGlyphSection(baseline - top, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 0, 0, 0, false);
    FT_Done_Glyph(glyph);

    auto img1 = Magick::Image(
        GlyphDimention,
        GlyphDimention,
        "RGBA",
        Magick::StorageType::CharPixel,
        pixels.data()
    );

    for(auto &px: pixels) {
        px = 0;
    }

    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
        std::cerr << "Glyph not loaded.";
        return std::nullopt;
    }
    FT_Get_Glyph(face->glyph, &glyph);
    if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true)) {
        std::cerr << "Glyph not rendered.";
        return std::nullopt;
    }
    bmpGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    int glyphLeft = bmpGlyph->left;
    top = bmpGlyph->top;
    RenderGlyphSection(baseline - top, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 255, 255, 255, true, glyphLeft-borderLeft);
    FT_Done_Glyph(glyph);

    auto img2 = Magick::Image(
        GlyphDimention,
        GlyphDimention,
        "RGBA",
        Magick::StorageType::CharPixel,
        pixels.data()
    );
    img1.composite(img2, 0, 0, Magick::AtopCompositeOp);
//    img1.write(std::string{c} + ".png");

    return img1;
}

bool DrawString(const std::string& str)
{
    if (FT_Set_Pixel_Sizes(face, 0, GlyphDimention)) {
        std::cerr << "Face size not set.";
        return false;
    }

    auto clr = Magick::Color(0, 0, 0, QuantumAlphaTransparent);
    Magick::Image outImage(
        Magick::Geometry(GlyphDimention*str.length(), GlyphDimention),
        clr
    );
    Magick::Image bgImage(
        Magick::Geometry(GlyphDimention*str.length(), GlyphDimention),
        Magick::Color(QuantumMax, 0, QuantumMax, QuantumMax - QuantumAlphaTransparent)
    );

    int i = 0;
    for (auto c : str) {
        auto img = DrawGlyph(c);
        if (!img) {
            return false;
        }
        bgImage.composite(*img, i*GlyphDimention, 0, Magick::AtopCompositeOp);
        ++i;
    }
    bgImage.alphaChannel(MagickCore::AlphaChannelType::DeactivateAlphaChannel);
    bgImage.write("test.png");
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }
    if (FT_Init_FreeType( &library )) {
        std::cerr << "Freetype library not opened.";
        return 1;
    }
    Magick::InitializeMagick(argv[0]);
    if (FT_New_Face(
        library,
        argv[1],
        0,
        &face
    )) {
        std::cerr << "Face not created.";
        return 1;
    }
    FT_Stroker_New(library, &stroker);
    FT_Stroker_Set(stroker, borderPointSize, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    if (!DrawString("cCqQABDabd")) {
        return 1;
    }

    FT_Stroker_Done(stroker);
    if (FT_Done_Face(face)) {
        std::cerr << "Face not freed.";
        return 1;
    }
    FT_Done_FreeType(library);
    return 0;
}
