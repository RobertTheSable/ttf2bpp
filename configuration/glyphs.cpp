#include "ttf2bpp/glyphs.h"
#include "config_impl.h"
#include "utf_conv.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>

#include <boost/locale.hpp>
#include <boost/locale/localization_backend.hpp>
#include <utf8.h>
#ifdef TTF_SET_ICU_DATA_DIR
#include <unicode/putil.h>
#endif

namespace ttf2bpp {
    void setupDataDir(const std::string& path)
    {
#ifdef TTF_SET_ICU_DATA_DIR
        u_setDataDirectory(path.c_str());
#endif
    }

    std::optional<unsigned long> fromUtf8(const std::string& utf8)
    {
        std::string normalized{""};
        try {
            auto locale = boost::locale::generator().generate("en_US.utf-8");
            normalized = boost::locale::normalize(
                utf8,
                boost::locale::norm_nfc,
                locale
            );
        } catch (std::exception &e) {
            std::cerr << e.what() << '\n';
            return std::nullopt;
        }

        if (!utf8::is_valid(normalized)) {
            return std::nullopt;
        }
        auto utf32Str = utf8::utf8to32(normalized);

        if (utf32Str.length() != 1) {
            return std::nullopt;
        }
        return (unsigned long)utf32Str.front();
    }

    std::string toUtf8(unsigned long utf32)
    {
        return utf8::utf32to8(std::u32string{(char32_t)utf32});
    }
    std::vector<unsigned long> readGlyphsFromPaintext(const std::string& filepath)
    {
        if (!std::filesystem::exists(std::filesystem::path(filepath))) {
            return {};
        }
        std::ifstream inFile(filepath);
        if (!inFile) {
            return {};
        }
        std::vector<unsigned long> result;

        std::string line = "";

        while (std::getline(inFile, line)) {
            for (auto it = line.begin(); it != line.end() ; utf8::next(it, line.end())) {
                auto next = it;
                auto cp = utf8::next(next, line.end());

                if (auto str = std::string(it, next);
                        str == "\t" ||  str == "\n" || str== "\r" || str == "\u00A0") {
                    continue;
                }
                result.push_back((unsigned long)cp);
            }
        }

        return result;
    }
}
