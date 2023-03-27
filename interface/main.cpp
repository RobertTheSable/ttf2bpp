#include <iostream>
#include "ttf2bpp/ttf2bpp.h"
#include <cxxopts.hpp>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }

    std::string fontpath = "";
    std::string output = "";
    std::vector<std::string> blocks{"ASCII"};
    if (*(argv[1]) == '-') {
        cxxopts::Options programOptions(
                        argv[0],
                    "Program to create a 2BPP font front a Freetype-compatible font.");
        programOptions.add_options()
                (
                    "u,unicode-block",
                    "Generate text based on the specified Unicode block.\n"
                    "Must match the name from the Unicode Character Database.",
                    cxxopts::value<std::vector<std::string>>(),
                    "BLOCK"
                )
                ("h,help", "Show this message.")
                ("font-file","Font file to parse",  cxxopts::value<std::string>())
                ("output-file","Output File (Optional)",  cxxopts::value<std::string>());
        programOptions.parse_positional({"font-file", "output-file"});
        programOptions.positional_help("[font-file] (output-file)");
        auto result = programOptions.parse(argc, argv);

        if (!result.count("font-file") || result.count("help")) {
            std::cout << programOptions.help() << '\n';
            return 0;
        } else {
            fontpath = result["font-file"].as<std::string>();
        }
        if (result.count("output-file")) {
            output = result["output-file"].as<std::string>();
        }
        if (result.count("unicode-block")) {
            blocks = result["unicode-block"].as<std::vector<std::string>>();
        }
    } else {
        fontpath = argv[1];
    }

    auto config = ttf2bpp::readConfiguration("config.yml");

    try {
        auto db = ttf2bpp::UnicodeDatabase::getUnicodeDatabase("ucd.all.grouped.xml", std::move(blocks));
        auto sorted = config.arrange(db.codes);
        auto finder =[&config] (ttf2bpp::Configuration::Colors clr) -> std::optional<int> {
             auto res = std::find(config.ordering.begin(), config.ordering.end(), clr);
             if (res != std::end(config.ordering)) {
                 return res - std::begin(config.ordering);
             }
             return std::nullopt;
        };
        ttf2bpp::ColorIndexes idxs{
            .background = finder(ttf2bpp::Configuration::Colors::Background).value_or(3),
            .text = finder(ttf2bpp::Configuration::Colors::Text).value_or(2),
            .border1 = finder(ttf2bpp::Configuration::Colors::Border).value_or(1),
            .border2 = finder(ttf2bpp::Configuration::Colors::Unused).value_or(0),
        };

        ttf2bpp::Renderer render(13, 112, 40, idxs, argv[0], fontpath);
        if (!(bool)render) {
            std::cerr << "renderer is invalid.";
            return 2;
        }

        auto configData = render.render(std::span<unsigned long>(sorted.begin(), sorted.size()), config.getOutputPath(fontpath, output));
        ttf2bpp::writeFontData("test.yml", configData);
        ttf2bpp::writeConfiguration("config.yml", config);
    } catch(std::runtime_error &e) {
        std::cerr << "Output failed: " << e.what() << '\n';
        return 1;
    }


    return 0;
}
