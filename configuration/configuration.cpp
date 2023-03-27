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
    auto defConfiguration = Configuration{};
    return defConfiguration;

}
TTF_BPP_EXPORT void writeConfiguration(const std::string& path, Configuration config)
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

configuration::configuration()=default;

TTF_BPP_EXPORT std::vector<GlyphInput> ttf2bpp::Configuration::arrange(const std::vector<unsigned long> &input) const
{
    if (input.empty()) {
        throw std::runtime_error("No input glyphs provided.");
    }
    std::vector<GlyphInput> output;
    auto source = input.begin();
    int idx = glyphStart;
    for (auto& r: reservedGlyphs) {
        while(idx < r.index) {
            if (reservedGlyphs.find(Reserved{.code = *source, .search = true}) == reservedGlyphs.end()) {
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
    return fspath(in).filename().replace_extension(fspath(extension)).string();
}

void Configuration::writeFontData(const std::vector<GlyphData> &data) const
{
    using fspath = std::filesystem::path;
    auto basename = fspath(encFileName).stem().string();
    std::ofstream output(encFileName);
    YAML::Node n;
    for (auto& gl: data) {
        auto gl2 = gl;
        gl2.code += glyphStart;
        n[gl2.utf8Repr] = gl2;
    }
    YAML::Node root;
    root[basename]["Encoding"] = n;
    output << root;
    output.close();
}

Renderer Configuration::getRenderer(ColorIndexes palette, const std::string &file) const
{
    using fspath = std::filesystem::path;
    if (fspath(file).extension() == ".png") {
        return Renderer(palette);
    }
    return Renderer(baseline, alphaThreshold, borderPointSize, palette, file, renderWidth);
}

Configuration& Configuration::updateOutputParams(const std::string &fontFile, const std::string &encFile)
{
    using fspath = std::filesystem::path;
    if (encFile != "") {
        encFileName = encFile;
        encNameSet = true;
        return *this;
    }
    encNameSet = false;
    encFileName = fspath(fontFile).filename().replace_extension(fspath(".yml")).string();
    return *this;
}

} // namespace ttf2bpp
