#include "ttf2bpp/renderer.h"
#include "render_glyph_conv.h"
#include <stdexcept>
#include <span>
#include <vector>
#include <fstream>

#include <Magick++.h>
#include <freetype/freetype.h>

#include FT_STROKER_H

namespace ttf2bpp {

constexpr const unsigned int QuantumMax = (1 << MAGICKCORE_QUANTUM_DEPTH) - 1;
constexpr const unsigned int QuantumAlphaTransparent = (MAGICKCORE_HDRI_ENABLE == 0) ? QuantumMax : 0;
constexpr const unsigned int QuantumAlphaOpaque = QuantumMax - QuantumAlphaTransparent;
constexpr const unsigned int GlyphDimention = 16;

namespace  {
    struct libUnload {
        void operator()(FT_Library lib) {
            FT_Done_FreeType(lib);
        };
    };

    bool magickLoaded = false;
    std::shared_ptr<FT_LibraryRec_> library;
    auto loadLibrary() {
        if (!library) {
            FT_Library lib;
            if (FT_Init_FreeType(&lib)) {
                throw std::runtime_error("Freetype library not loaded.");
            }
            library = std::shared_ptr<FT_LibraryRec_>(lib, libUnload{});
        }
        return library;
    }
    auto BackgroundColor = Magick::Color(QuantumMax, 0, QuantumMax, QuantumAlphaOpaque);
    auto TextColor = Magick::Color(QuantumMax, QuantumMax, QuantumMax, QuantumAlphaOpaque);
    auto BorderColor = Magick::Color(0, 0, 0, QuantumAlphaOpaque);
}

struct RenderResult {
    Magick::Image img;
    int width;
};

struct Renderer::pImpl {
    int         baseline = 13;
    int         alphaThreshold = 112;
    int         borderPointSize = 40;
    FT_Face     face;
    FT_Stroker  stroker;
    ColorIndexes indexes;
    ~pImpl() {
        FT_Stroker_Done(stroker);
        FT_Done_Face(face);
    }
    bool RenderGlyphSection(int top, int left, FT_Bitmap bitmap, std::span<unsigned char> pixels, unsigned char red, unsigned char green, unsigned char blue, bool keepOpacity) {
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
                    opacity = (value >= alphaThreshold) ? 255 : 0;
                } else {
                    opacity = (value == 0) ? 0 : 255;
                }

                for (auto channel : {red, green, blue, opacity}) {
                    *px = channel;
                    ++px;
                }
            }
        }
        return true;
    }

    RenderResult DrawGlyph(FT_ULong c)
    {
        int width = 0;
        auto glyph_index = FT_Get_Char_Index(face, c);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            throw std::runtime_error("Glyph not loaded.");
        }
        auto metrics = face->glyph->metrics;

        FT_Glyph glyph;
        FT_Get_Glyph(face->glyph, &glyph);
        FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
        if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true)) {
            throw std::runtime_error("Glyph border not rendered.");
        }
        auto bmpGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
        int top = bmpGlyph->top;
        int borderLeft = bmpGlyph->left;
        width = bmpGlyph->bitmap.width;
        std::vector<unsigned char> pixels(GlyphDimention * GlyphDimention * 4, 0);
        RenderGlyphSection(baseline - top, 0, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 0, 0, 0, false);
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
            throw std::runtime_error("Glyph not loaded.");
        }
        FT_Get_Glyph(face->glyph, &glyph);
        if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true)) {
            throw std::runtime_error("Glyph not rendered.");
        }
        bmpGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
        int glyphLeft = bmpGlyph->left;
        top = bmpGlyph->top;
        RenderGlyphSection(baseline - top, glyphLeft-borderLeft, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 255, 255, 255, true);
        FT_Done_Glyph(glyph);

        auto img2 = Magick::Image(
            GlyphDimention,
            GlyphDimention,
            "RGBA",
            Magick::StorageType::CharPixel,
            pixels.data()
        );
        img1.composite(img2, 0, 0, Magick::AtopCompositeOp);

        return RenderResult{
            .img = img1,
            .width = width
        };
    }


    auto outputBppRow(Magick::Image bgImage, int y)
    {
        const int quarterSize = GlyphDimention/2;
        std::vector<unsigned char> data(16*2, 0);
        auto outPtr = data.begin();
        for (int x = 0; x < 2 ; ++x) {
            auto pixelPtr = bgImage.getConstPixels(x*quarterSize, y*quarterSize, quarterSize, quarterSize);
            for (int qY = 0; qY < quarterSize; ++qY) {
                for (int qX = 0; qX < quarterSize; ++qX) {
                    auto packet = *pixelPtr++;
                    auto pxColor = Magick::Color(packet.red, packet.green, packet.blue, QuantumAlphaOpaque);
                    unsigned char bitmask1 = 0, bitmask2 = 0;
                    if (pxColor == BackgroundColor) {
                        bitmask1 = indexes.background & 1;
                        bitmask2 = (indexes.background & 2) >> 1;
                    } else if (pxColor == TextColor) {
                        bitmask1 = indexes.text & 1;
                        bitmask2 = (indexes.text & 2) >> 1;
                    } else if (pxColor == BorderColor) {
                        bitmask1 = indexes.border1 & 1;
                        bitmask2 = (indexes.border1 & 2) >> 1;
                    } else {
                        throw std::runtime_error("Unrecognized color.");
                    }
                    *outPtr |= (bitmask1 << (7 - qX));
                    *(outPtr+1) |= (bitmask2 << (7 - qX));
                } //qx
                ++outPtr;
                ++outPtr;
            } //qy
        } // x
        return data;
    }

    void appendBppRowsData(std::vector<Magick::Image> row, std::vector<unsigned char>& data)
    {
        for (auto y = 0; y < 2; ++y) {
            for (auto& rowImg: row) {
                auto rowData = outputBppRow(rowImg, y);
                data.reserve(rowData.size());
                data.insert(std::end(data), std::begin(rowData), std::end(rowData));
            }
        }
    }
};

Renderer::Renderer(
    int baseline,
    int alphaThreshold,
    int borderPointSize,
    ColorIndexes indexes,
    const std::string& workPath,
    const std::string& facePath
) {
    _valid = false;
    _impl = std::make_unique<pImpl>(baseline, alphaThreshold, borderPointSize);
    _impl->indexes = indexes;
    auto library = loadLibrary();

    if (!magickLoaded) {
        Magick::InitializeMagick(workPath.c_str());
        magickLoaded = true;
    }

    if (FT_New_Face(
        library.get(),
        facePath.c_str(),
        0,
        &_impl->face
    )) {
        throw std::runtime_error("Face not created.");
    }
    FT_Stroker_New(library.get(), &_impl->stroker);
    FT_Stroker_Set(_impl->stroker, _impl->borderPointSize, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    _valid = true;
}

Renderer::~Renderer()=default;

Renderer::operator bool() const
{
    return _valid;
}

std::vector<GlyphData> Renderer::render(std::span<unsigned long> glyphs, const std::string& filename, bool image)
{
    if (FT_Set_Pixel_Sizes(_impl->face, 0, GlyphDimention)) {
        throw std::runtime_error("Face size not set.");
    }

    std::vector<GlyphData> gData;
    if (image) {
        Magick::Image bgImage(
            Magick::Geometry(GlyphDimention*glyphs.size(), GlyphDimention),
            Magick::Color(QuantumMax, 0, QuantumMax, QuantumAlphaOpaque)
        );

        int i = 0;
        for (auto c : glyphs) {
            auto result = _impl->DrawGlyph(c);
            bgImage.composite(result.img, i*GlyphDimention, 0, Magick::AtopCompositeOp);
            ++i;
            gData.push_back(getGlyphData(c, result.width, i));
        }
        bgImage.alphaChannel(MagickCore::DeactivateAlphaChannel);

        bgImage.write(filename);
    } else {
        std::vector<unsigned char> data;
        std::vector<Magick::Image> row;
        int i = 0;
        for (auto c : glyphs) {
            Magick::Image bgImage(
                Magick::Geometry(GlyphDimention, GlyphDimention),
                Magick::Color(QuantumMax, 0, QuantumMax, QuantumAlphaOpaque)
            );

            auto result =  _impl->DrawGlyph(c);
            bgImage.composite(result.img, 0, 0, Magick::AtopCompositeOp);
            row.push_back(bgImage);
            if (row.size() == 8) {
                _impl->appendBppRowsData(row, data);
                row.clear();
            }
            gData.push_back(getGlyphData(c, result.width, i));
            ++i;
        }
        if (!row.empty()) {
            while(row.size() != 8) {
                row.push_back(Magick::Image("16x16", BackgroundColor));
            }
            _impl->appendBppRowsData(row, data);
        }
        std::ofstream output(filename, std::ios_base::binary);
        output.write((char*)data.data(), data.size());
        output.flush();
        output.close();
    }

    return gData;
}

} // namespace ttf2bpp
