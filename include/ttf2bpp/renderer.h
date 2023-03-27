#ifndef TTF2BPP_RENDERER_H
#define TTF2BPP_RENDERER_H
#include "ttf2bpp_exports.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <span>

namespace ttf2bpp {

struct TTF_BPP_EXPORT ColorIndexes {
    int background = 3, text = 2, border1 = 1, border2 = 0;
};

struct TTF_BPP_EXPORT GlyphData {
    std::string utf8Repr;
    int code;
    int length;
};

constexpr const unsigned int BaseGlyphDimention = 8;
constexpr const unsigned int GlyphDimention = 16;

class TTF_BPP_EXPORT Renderer
{
    struct pImpl;
    std::unique_ptr<pImpl> _impl;
    bool _valid;
    std::string _inFileName;
public:
    Renderer(
        int baseline,
        int alphaThreshold,
        int borderPointSize,
        ColorIndexes indexes,
        const std::string& facePath,
        int renderWidth
    );
    Renderer(ColorIndexes indexes);
    ~Renderer();
    explicit operator bool() const;
    std::vector<GlyphData> render(std::span<unsigned long> glyphs, const std::string& filename, int glyphWidth);
    void encodeImage(const std::string& inFile, const std::string outfile) const;
    bool imageMode() const;
};

} // namespace ttf2bpp

#endif // TTF2BPP_RENDERER_H
