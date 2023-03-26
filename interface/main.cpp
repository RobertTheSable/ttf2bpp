#include <iostream>
#include "ttf2bpp/ttf2bpp.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }
    auto config = ttf2bpp::readConfiguration("config.yml");

    auto db = ttf2bpp::UnicodeDatabase::getUnicodeDatabase("ucd.all.grouped.xml", "ASCII", "");

    ttf2bpp::Renderer render(13, 112, 40, {3, 2, 1}, argv[0], argv[1]);

    auto sorted = config.arrange(db.codes);
    auto configData = render.render(std::span<unsigned long>(sorted.begin(), sorted.size()), "test.smc", false);
    ttf2bpp::writeFontData("test.yml", configData);
    ttf2bpp::writeConfiguration("config.yml", config);
    return 0;
}
