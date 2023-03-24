#include <iostream>
#include <freetype/freetype.h>
#include <Magick++.h>
#include <optional>

constexpr const unsigned int QuantumMax = (1 << MAGICKCORE_QUANTUM_DEPTH) - 1;

namespace  {
    FT_Library library;
    FT_Face     face;
}

std::optional<Magick::Image> DrawGlyph(char c)
{
    auto glyph_index = FT_Get_Char_Index(face, c);
    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
        std::cerr << "Glyph not loaded.";
        return std::nullopt;
    }
    if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
        std::cerr << "Glyph not rendered.";
        return std::nullopt;
    }
    auto bitmap = face->glyph->bitmap;
    int top = face->glyph->bitmap_top;

    std::vector<unsigned char> pixels(bitmap.width * bitmap.rows * 4);

    for (int y = 0; y < bitmap.rows; ++y) {
        for (int x = 0; x < bitmap.width; ++x) {
            int realy = y; // -top
            if (realy < 0) {
                continue;
            }
            int target = x + (y * bitmap.width);

            auto px = pixels.begin()+(target*4), pxend = px+4;
//            unsigned char opacity = (bitmap.buffer[target] == 0) ? 0 : 255;
            while (px != pxend) {
                *px = bitmap.buffer[target];
                ++px;
            }
//            *px = opacity;
        }
    }
    return Magick::Image(
        bitmap.width,
        bitmap.rows,
        "RGBO",
        Magick::StorageType::CharPixel,
        pixels.data()
    );
}

bool DrawString(const std::string& str)
{
    if (FT_Set_Pixel_Sizes(face, 0, 32)) {
        std::cerr << "Face size not set.";
        return false;
    }

    auto clr = Magick::Color(0, 0, 0, 0);
    Magick::Image outImage(
        Magick::Geometry(32*str.length(), 32),
        clr
    );

    int i = 0;
    for (auto c : str) {
        auto img = DrawGlyph(c);
        if (!img) {
            return false;
        }
        outImage.composite(*img, i*32, 0, Magick::CopyCompositeOp);
        ++i;
    }
    Magick::Image bgImage(
        Magick::Geometry(32*str.length(), 32),
        Magick::Color(QuantumMax, 0, QuantumMax, QuantumMax)
    );
    outImage.quantizeColors(4);
    outImage.quantize();
    bgImage.composite(outImage, 0, 0, Magick::AtopCompositeOp);
//    out
    bgImage.write("test.png");
    return true;
}

int main(int argc, char* argv[])
{
    if (FT_Init_FreeType( &library )) {
        std::cerr << "Freetype library not opened.";
        return 1;
    }
    Magick::InitializeMagick(argv[0]);
    if (FT_New_Face(
        library,
        "/usr/share/fonts/truetype/Courier.ttf",
        0,
        &face
    )) {
        std::cerr << "Face not created.";
        return 1;
    }

    if (!DrawString("qQABCDabcd")) {
        return 1;
    }

    if (FT_Done_Face(face)) {
        std::cerr << "Face not freed.";
        return 1;
    }
    FT_Done_FreeType(library);
    return 0;
}
