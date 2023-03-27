#include <iostream>
#include "ttf2bpp/ttf2bpp.h"
#include <cxxopts.hpp>

struct glyphDataResult {
    std::optional<int> spaceCode = std::nullopt;
    std::vector<unsigned long> glyphs;
};

auto getData(bool useUnicode, const std::string& glyphFile, std::vector<std::string>&& blocks)
{
    if (useUnicode) {
        auto db = ttf2bpp::UnicodeDatabase::getUnicodeDatabase("ucd.all.grouped.xml", std::move(blocks));
        if (db.codes.empty()) {
            throw std::runtime_error("No input was provided to generate the font.");
        }
        return db.codes;
    }
    return ttf2bpp::readGlyphsFromPaintext(glyphFile);
}

int main(int argc, char* argv[])
{
    std::string fontpath = "";
    std::string output = "";
    std::string glyphFile = "glyphs.txt";
    std::string encFile = "";
    std::vector<std::string> blocks{};
    bool useUnicode = false;
    if (argc == 1 || *(argv[1]) == '-') {

        cxxopts::Options programOptions(
                        argv[0],
                    "Program to create a 2BPP font front a Freetype-compatible font.");
        programOptions.add_options()
                (
                    "g,glyph-file",
                    "Generate text based on the specified plaintext file.\n"
                    "Default: glyphs.yml",
                    cxxopts::value<std::string>(),
                    "FILE"
                )
                (
                    "u,unicode-block",
                    "Generate text based on the specified Unicode block.\n"
                    "Must match the name from the Unicode Character Database.\n"
                    "Overrides the glyph-file options.",
                    cxxopts::value<std::vector<std::string>>(),
                    "BLOCK"
                )
                (
                    "e,encoding-file",
                    "File in which to store generated encoding data. Default: (font name).yml ",
                    cxxopts::value<std::string>(),
                    "FILE"
                )
                ("h,help", "Show this message.")
                ("font-file","Font file to parse",  cxxopts::value<std::string>())
                ("output-file","Output File (Optional)",  cxxopts::value<std::string>());
        programOptions.parse_positional({"font-file", "output-file"});
        programOptions.positional_help("[font-file] (output-file)");

        try {
            auto result = programOptions.parse(argc, argv);

            bool inputProvided = (result.count("unicode-block") || result.count("glyph-file") || result.count("encoding-file"));
            if (!result.count("font-file") || !inputProvided || result.count("help")) {
                std::cout << programOptions.help() << '\n';
                return 0;
            } else {
                fontpath = result["font-file"].as<std::string>();
            }
            if (result.count("output-file")) {
                output = result["output-file"].as<std::string>();
            }
            if (result.count("glyph-file")) {
                glyphFile = result["glyph-file"].as<std::string>();
            } else if (result.count("unicode-block")) {
                useUnicode = true;
                blocks = result["unicode-block"].as<std::vector<std::string>>();
            }
            if (result.count("encoding-file")) {
                encFile = result["encoding-file"].as<std::string>();
            }
        } catch (cxxopts::OptionParseException& e) {
            std::cout << e.what() << '\n';
            std::cout << programOptions.help() << '\n';
            return 0;
        }
    } else {
        fontpath = argv[1];
        if (argc == 3) {
            output = argv[2];
        }
    }

    auto config = ttf2bpp::readConfiguration("config.yml").updateOutputParams(fontpath, encFile);

    try {
        auto sorted = config.arrange(getData(useUnicode, glyphFile, std::move(blocks)));

        auto finder =[&config] (ttf2bpp::Configuration::Colors clr) -> std::optional<int> {
             auto res = std::find(config.ordering.begin(), config.ordering.end(), clr);
             if (res != std::end(config.ordering)) {
                 return res - std::begin(config.ordering);
             }
             return std::nullopt;
        };

        ttf2bpp::ColorIndexes palette{
            .background = finder(ttf2bpp::Configuration::Colors::Background).value_or(3),
            .text = finder(ttf2bpp::Configuration::Colors::Text).value_or(2),
            .border1 = finder(ttf2bpp::Configuration::Colors::Border).value_or(1),
            .border2 = finder(ttf2bpp::Configuration::Colors::Unused).value_or(0),
        };
        ttf2bpp::Renderer render(config.baseline, config.alphaThreshold, config.borderPointSize, palette, fontpath);
        if (!(bool)render) {
            std::cerr << "renderer is invalid.";
            return 2;
        }

        auto configData = render.render(std::span<unsigned long>(sorted.begin(), sorted.size()), config.getOutputPath(fontpath, output));

        config.writeFontData(configData);
        ttf2bpp::writeConfiguration("config.yml", config);
        std::cout << "Font generated successfully.\n";
    } catch(std::runtime_error &e) {
        std::cerr << "Output failed: " << e.what() << '\n';
        return 1;
    }


    return 0;
}
