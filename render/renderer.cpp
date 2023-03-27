#include "ttf2bpp/renderer.h"
#include "render_glyph_conv.h"
#include <stdexcept>
#include <span>
#include <vector>
#include <fstream>
#include <filesystem>

//#include <Magick++.h>
#include <freetype/freetype.h>
#include "pnghelper.h"
#include <png++/png.hpp>

#include FT_STROKER_H

namespace ttf2bpp {

//constexpr const unsigned int QuantumMax = (1 << MAGICKCORE_QUANTUM_DEPTH) - 1;
//constexpr const unsigned int QuantumAlphaTransparent = (MAGICKCORE_HDRI_ENABLE == 0) ? QuantumMax : 0;
//constexpr const unsigned int QuantumAlphaOpaque = QuantumMax - QuantumAlphaTransparent;

namespace  {
//    bool magickLoaded = false;
    auto BackgroundColor = png::rgba_pixel{0xFF, 0, 0xFF, 0xFF};
    auto TextColor = png::rgba_pixel{0xFF, 0xFF, 0xFF, 0xFF};
    auto BorderColor = png::rgba_pixel{0, 0, 0, 0xFF};
}

struct RenderResult {
    png::image<png::rgba_pixel> img;
    int width;
};

class Renderer::pImpl {
    bool strokerLoaded = false;
    bool faceLoaded = false;
    bool imageMode = false;
public:
    int         baseline = 13;
    int         alphaThreshold = 112;
    int         borderPointSize = 40;
    std::string facePath;
    FT_Face     face;
    FT_Stroker  stroker;
    ColorIndexes indexes;
    FT_Library lib;
    pImpl(ColorIndexes pal)
    {
        indexes = pal;
        imageMode = true;
    }
    pImpl(
        int b,
        int a,
        int bd,
        const std::string& fPath,
        int renderSize
    ): baseline{b}, alphaThreshold{a}, borderPointSize{bd}, facePath{fPath} {
        if (FT_Init_FreeType(&lib)) {
            throw std::runtime_error("Freetype library not loaded.");
        }
        if (FT_New_Face(
            lib,
            facePath.c_str(),
            0,
            &face
        )) {
            throw std::runtime_error("Face not created.");
        }
        faceLoaded = true;
        if (FT_Set_Pixel_Sizes(face, 0, renderSize)) {
            throw std::runtime_error("Face size not set.");
        }
        if (FT_Stroker_New(lib, &stroker)) {
            throw std::runtime_error("Stroker not loaded.");
        }
        FT_Stroker_Set(stroker, borderPointSize, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
        strokerLoaded = true;
    }
    ~pImpl() {
        if (strokerLoaded) {
            FT_Stroker_Done(stroker);
        }
        if (faceLoaded) {
            FT_Done_Face(face);
        }
        FT_Done_FreeType(lib);
    }
    bool inPictureMode()
    {
        return imageMode;
    }
    bool RenderGlyphSection(int top, int left, FT_Bitmap bitmap, std::span<unsigned char> pixels, unsigned char red, unsigned char green, unsigned char blue, bool keepOpacity, int width) {
        for (int y = 0; y < bitmap.rows; ++y) {
            for (int x = 0; x < bitmap.width; ++x) {
                int realy = y + top;
                int realx = x + left;
                if (realy < 0 || realy >= GlyphDimention) {
                    continue;
                }
                int target = realx + (realy * width);
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

    RenderResult DrawGlyph(FT_ULong c, int imageWidth)
    {
        int width = 0;
        auto glyph_index = FT_Get_Char_Index(face, c);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            throw std::runtime_error("Glyph not loaded.");
        }
        auto advance = face->glyph->advance.x;

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
        std::vector<unsigned char> pixels(imageWidth * GlyphDimention * 4, 0);
        RenderGlyphSection(baseline - top, 0, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 0, 0, 0, false, imageWidth);

        FT_Done_Glyph(glyph);

        png::image<png::rgba_pixel> img1 = buildPng({pixels.begin(), pixels.size()}, imageWidth, GlyphDimention);

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
        RenderGlyphSection(baseline - top, glyphLeft-borderLeft, bmpGlyph->bitmap, std::span(pixels.begin(), pixels.end()), 255, 255, 255, true, imageWidth);
        FT_Done_Glyph(glyph);

        auto img2 = buildPng({pixels.begin(), pixels.size()}, imageWidth, GlyphDimention);

        composite(img1, img2, 0, 0);

        if (width == 0) {
            width = (advance >> 6);
        }

        return RenderResult{
            .img = img1,
            .width = width
        };
    }


    auto outputBppRow(png::image<png::rgba_pixel> bgImage, int y)
    {
        const int quarterSize = GlyphDimention/2;
        std::vector<unsigned char> data(16*2, 0);
        auto outPtr = data.begin();
        for (int x = 0; x < 2 ; ++x) {
            auto quarterimage = buildPng(bgImage, x*quarterSize, y*quarterSize, quarterSize, quarterSize);
            for (int qY = 0; qY < quarterSize; ++qY) {
                for (int qX = 0; qX < quarterSize; ++qX) {
                    auto packet = quarterimage.get_pixel(qX, qY);
                    unsigned char bitmask1 = 0, bitmask2 = 0;
                    if (equals(packet, BackgroundColor)) {
                        bitmask1 = indexes.background & 1;
                        bitmask2 = (indexes.background & 2) >> 1;
                    } else if (equals(packet, TextColor)) {
                        bitmask1 = indexes.text & 1;
                        bitmask2 = (indexes.text & 2) >> 1;
                    } else if (equals(packet, BorderColor)) {
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

    void appendBppRowsData(std::vector<png::image<png::rgba_pixel>> row, std::vector<unsigned char>& data)
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
    ColorIndexes idxs,
    const std::string& facePath,
    int renderWidth
) {
    if (idxs.background == idxs.border1 || idxs.background == idxs.text || idxs.border1 == idxs.text) {
        throw std::runtime_error("Color ordering cannot contain duplicates.");
    }
    _valid = false;
    _inFileName = facePath;
    _impl = std::make_unique<pImpl>(baseline, alphaThreshold, borderPointSize, facePath, renderWidth);
    _impl->indexes = idxs;
    _valid = true;
}

Renderer::Renderer(ColorIndexes indexes)
{
    _impl = std::make_unique<pImpl>(indexes);
}

Renderer::~Renderer()=default;

Renderer::operator bool() const
{
    return _valid || _impl->inPictureMode();
}

std::vector<GlyphData> Renderer::render(std::span<GlyphInput> glyphs, const std::string& filename, int glyphWidth)
{
    using fspath = std::filesystem::path;
    if (!_valid) {
        return {};
    }
    bool image = false;
    std::string outfilepath = filename;
    if (filename == "") {
        outfilepath = fspath(_inFileName).replace_extension(fspath(".bin")).string();
    } else {
        if (auto ext = fspath(filename).extension().string(); ext == ".png" ) {
            image = true;
        } else if (ext == ".jpeg" || ext == ".jpg" || ext == ".bmp") {
            throw std::runtime_error("Only PNG image output is supported.");
        }
    }

    std::vector<GlyphData> gData;
    auto append = [&gData] (GlyphInput in, int width, int code) -> int {
        if (width != 0) {
            if (in.label == "") {
                gData.push_back(getGlyphData(in.utf32code, width, code));
            } else {
                gData.push_back(GlyphData{.utf8Repr = in.label, .code = code, .length = width});
            }

            return code+1;
        }
        return code;
    };


    int rowSize = 128 / glyphWidth;
    if (image) {
        int i = 0;
        int rows = 0;
        for (int count = glyphs.size(); count > 0; count -= rowSize) {
            ++rows;
        }
        auto bgImage = buildPng(BackgroundColor, glyphWidth*rowSize, GlyphDimention*rows);
        for (auto c : glyphs) {
            int x = i%rowSize, y = i/rowSize;
            auto result = _impl->DrawGlyph(c.utf32code, glyphWidth);
            composite(bgImage, result.img, x*glyphWidth, y*GlyphDimention);
            i = append(c, result.width, i);
        }

        bgImage.write(outfilepath);
    } else {
        std::vector<unsigned char> data;
        std::vector<png::image<png::rgba_pixel>> row;
        int i = 0;
        for (auto c : glyphs) {
            auto bgImage = buildPng(BackgroundColor, glyphWidth, GlyphDimention);

            auto result =  _impl->DrawGlyph(c.utf32code, glyphWidth);
            composite(bgImage, result.img, 0, 0);
            row.push_back(bgImage);
            if (row.size() == 8) {
                _impl->appendBppRowsData(row, data);
                row.clear();
            }
            i = append(c, result.width, i);
        }
        if (!row.empty()) {
            while(row.size() != rowSize) {
                row.push_back(buildPng(BackgroundColor, glyphWidth, GlyphDimention));
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

void Renderer::encodeImage(const std::string& inFile, const std::string outfile) const
{
    png::image<png::rgba_pixel> img(inFile);
    if (img.get_width() != GlyphDimention * 8) {
        throw std::runtime_error("Invalid input image");
    }
    int columns = 8;
    int rows = img.get_height() /GlyphDimention;
    std::vector<unsigned char> data;
    for (int y = 0; y < rows; ++y) {
        std::vector<png::image<png::rgba_pixel>> row;
        for (int x = 0; x < columns; ++x) {
            row.push_back(buildPng(img, x*GlyphDimention, y*GlyphDimention, GlyphDimention, GlyphDimention));
        }
        _impl->appendBppRowsData(row, data);
    }
    std::ofstream output(outfile, std::ios_base::binary);
    output.write((char*)data.data(), data.size());
    output.flush();
    output.close();
}

bool Renderer::imageMode() const
{
    return _valid && _impl->inPictureMode();
}

} // namespace ttf2bpp
