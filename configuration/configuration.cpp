#include "config_impl.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <yaml-cpp/yaml.h>
namespace ttf2bpp {

TTF_BPP_EXPORT bool operator ==(const Reserved& lhs, const Reserved& rhs)
{
    return lhs.code == rhs.code;
}
TTF_BPP_EXPORT bool operator <(const Reserved& lhs, const Reserved& rhs)
{
    if (lhs.search || rhs.search) {
        return lhs.code < rhs.code;
    }
    return lhs.index < rhs.index;
}
TTF_BPP_EXPORT bool operator ==(const unsigned long& lhs, const Reserved& rhs)
{
    return lhs == rhs.code;
}
TTF_BPP_EXPORT bool operator ==(const Reserved& lhs, const unsigned long& rhs)
{
    return lhs.code == rhs;
}
TTF_BPP_EXPORT bool operator ==(const std::string& lhs, const Reserved& rhs)
{
    return lhs == rhs.label;
}
TTF_BPP_EXPORT bool operator ==(const Reserved& lhs, const std::string& rhs)
{
    return lhs.label == rhs;
}

TTF_BPP_EXPORT bool operator !=(const Reserved& lhs, const Reserved& rhs)
{
    return ! (lhs == rhs);
}

TTF_BPP_EXPORT Configuration readConfiguration(const std::string& path)
{
    if (std::filesystem::exists(std::filesystem::path(path))) {
        try {
            auto node = YAML::LoadFile(path);
            return node.as<Configuration>();
        } catch (YAML::Exception& e) {
            auto test = e.what();
            throw std::runtime_error(e.what());
        }
    }
    return Configuration{};

}

TTF_BPP_EXPORT void writeConfiguration(const std::string& path, const Configuration& config)
{
    if (std::filesystem::exists(std::filesystem::path(path))) {
        return;
    }
    std::ofstream output(path);
    YAML::Node outnode;
    outnode = config;
    output << outnode;
    output.close();
}

struct Configuration::Impl {
    int outputFontAsYAML(const options& conf, const std::vector<GlyphData> &data)
    {
        std::size_t max_val = 1 << (conf.fontByteWidth*8);
        int pageCount = (data.size() / max_val) + 1;
        using fspath = std::filesystem::path;
        auto basename = fspath(conf.encFileName).stem().string();
        auto ext = fspath(conf.encFileName).extension().string();
        if (ext != ".yaml") {
            ext = ".yml";
        }
        std::ofstream output(basename + ext);
        YAML::Node n;
        auto glyphItr = data.begin();
        for (auto& gl: data) {
            auto gl2 = gl;
            gl2.code += conf.glyphStart;
            if (gl2.code == max_val) {
                break;
            }
            n[gl2.utf8Repr] = gl2;
            ++glyphItr;
        }
        YAML::Node root;
        root[basename]["Encoding"] = n;

        int pageoffset = max_val;
        for (int pgIdx = 1; pgIdx < pageCount; ++ pgIdx) {
            YAML::Node page;
            while (glyphItr != data.end()) {
                auto gl2 = *glyphItr;
                gl2.code += conf.glyphStart;
                gl2.code -= (pageoffset - conf.glyphStart);
                if (gl2.code == max_val) {
                    break;
                }
                page[gl2.utf8Repr] = gl2;
                ++glyphItr;
            }
            pageoffset += max_val;
            root[basename]["Pages"].push_back(page);
        }
        output << root;
        output.close();
        return pageCount;
    }
    int outputFontAs64TASS(const options& conf, const std::vector<GlyphData> &data)
    {
        int pageoffset = 0;
        std::size_t max_val = 1 << (conf.fontByteWidth*8);
        int pageCount = (data.size() / max_val) + 1;
        using fspath = std::filesystem::path;
        auto basename = fspath(conf.encFileName).stem().string();

        // width and encoding should go in separate files for convenience.
        auto encPath = basename + "-enc.asm";

        std::ofstream encFile(encPath);
        encFile << ".enc \"" <<  basename << "\"\n" << std::hex;
        auto w = encFile.width();
        auto f = encFile.fill();
        for (auto glyph: data) {
            auto realVal = glyph.code + conf.glyphStart;
            encFile << ".edef \"" << glyph.utf8Repr << "\", ";
            if (conf.fontByteWidth == 1) {
                encFile << '$'
                        << std::setw(2) << std::setfill('0') << (realVal&0xFF)
                        << std::setw(w) << std::setfill(f) << '\n';
            } else {
                encFile <<'[';
                for (int count = 0; count < conf.fontByteWidth; ++count) {
                    encFile << '$'
                            << std::setw(2) << std::setfill('0') << (realVal &0xFF)
                            << std::setw(w) << std::setfill(f) << ',';
                    realVal >>= 8;
                }
                encFile.seekp(-1, std::ios_base::cur);

                encFile << "]\n";
            }
        }
        encFile.close();


        std::ofstream widthFile;

        if (conf.widthEncoding == WidthType::CSV) {
            widthFile.open(basename + "-width.csv");
            // TODO: do I need to add a header?
        } else {
            widthFile.open(basename + "-width.asm");
        }
        w = widthFile.width();
        f = widthFile.fill();
        auto glpyhItr = data.begin();
        while (glpyhItr != data.end()) {
            auto distance = data.end() - glpyhItr;
            int rowWidth = distance >= 8 ? 8 : distance;
            if (conf.widthEncoding == WidthType::TASS64) {
                widthFile << ".byte ";
            }
            for (int col = 0; col < rowWidth; ++col) {
                widthFile << std::setw(2) << std::setfill('0') <<  glpyhItr->length
                          << std::setw(w) << std::setfill(f) << ',';
                ++glpyhItr;
            }
            widthFile.seekp(-1, std::ios_base::cur);
            widthFile << '\n';
        }
        widthFile.close();

        return pageCount;
    }
};


Configuration::~Configuration()=default;

Configuration::Configuration()
    : _pImpl(new Impl) {

}

Configuration::Configuration(Configuration &&)=default;

TTF_BPP_EXPORT std::vector<GlyphInput> ttf2bpp::Configuration::arrange(const std::vector<unsigned long> &input) const
{
    if (input.empty()) {
        throw std::runtime_error("No input glyphs provided.");
    }
    std::vector<GlyphInput> output;
    auto source = input.begin();
    int idx = params.glyphStart;
    for (auto& r: params.reservedGlyphs) {
        while(idx < r.index) {
            if (params.reservedGlyphs.find(
                    Reserved{.code = *source, .search = true}
            ) == params.reservedGlyphs.end()) {
                output.push_back({.label = toUtf8(*source), .utf32code = *source});
                ++idx;
            }
            if (source+1 != input.end()) {
                ++source;
            }
        }
        output.push_back({.label = r.label, .utf32code = r.code});
        ++idx;
    }
    while (source != input.end()) {
        output.push_back({.label = toUtf8(*source), .utf32code = *source});
        ++source;
    }
    return output;
}

std::string Configuration::getOutputPath(const std::string &in, const std::string &out) const
{
    using fspath = std::filesystem::path;
    if (!out.empty()) {
        if (fspath(in).extension() == fspath(out).extension()) {
            throw std::runtime_error("Nothing to do - input and output format are the same.");
        }
        return out;
    }
    return fspath(in).filename().replace_extension(fspath(params.extension)).string();
}

int Configuration::writeFontData(const std::vector<GlyphData> &data) const
{
    if (params.outEncoding == EncType::YAML) {
        return _pImpl->outputFontAsYAML(params, data);
    } else {
        return _pImpl->outputFontAs64TASS(params, data);
    }

}

Renderer Configuration::getRenderer(ColorIndexes palette, const std::string &file) const
{
    using fspath = std::filesystem::path;
    if (fspath(file).extension() == ".png") {
        return Renderer(palette);
    }
    return Renderer(
        params.baseline,
        params.alphaThreshold,
        params.borderPointSize,
        palette,
        file,
        params.renderWidth
    );
}

auto Configuration::operator->() -> options*
{
    return &params;
}

auto Configuration::operator*() -> options&
{
    return params;
}

auto Configuration::operator->() const -> const options*
{
    return &params;
}

auto Configuration::operator*() const -> const options&
{
    return params;
}

Configuration& Configuration::updateOutputParams(const std::string& fontFile, const std::string& encFile)
{
    using fspath = std::filesystem::path;
    if (encFile != "") {
        params.encFileName = encFile;
        params.encNameSet = true;
        return *this;
    }
    params.encNameSet = false;

    if (params.encFileName == "") {
        params.encFileName = fspath(fontFile).stem().string();
    }

    return *this;
}

Configuration&& Configuration::changeOutputParams(const std::string &fontFile, const std::string &encFile)
{
    return std::move(this->updateOutputParams(fontFile, encFile));
}

} // namespace ttf2bpp
