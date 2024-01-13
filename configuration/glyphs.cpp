#include "ttf2bpp/glyphs.h"
#include "config_impl.h"
#include "iterator/iterator.h"
#include "utf_conv.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <boost/locale.hpp>
#include <boost/locale/localization_backend.hpp>
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
        auto locale = boost::locale::generator().generate("en_US.utf-8");
        auto normalized = boost::locale::normalize(
            utf8,
            boost::locale::norm_nfc,
            locale
        );
        auto utf32Str = boost::locale::conv::utf_to_utf<char32_t>(normalized);
        if (utf32Str.length() != 1) {
            return std::nullopt;
        }
        return (unsigned long)utf32Str.front();
    }

    std::string toUtf8(unsigned long utf32)
    {
        return boost::locale::conv::utf_to_utf<char>(std::basic_string<char32_t>{(char32_t)utf32});
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
        namespace ba=boost::locale::boundary;
        auto locale = boost::locale::generator().generate("en_US.utf-8");
        while (std::getline(inFile, line)) {
            ba::segment_index<std::string::const_iterator> map(ba::character, line.begin(), line.end(), locale);
            for (auto gl: map) {
                // TODO: See if there's non case-by-case way to handle formatting glyphs.
                if (auto str = gl.str(); (str == "\t" ||  str == "\n" || str== "\r" || str == "\u00A0")) {
                    continue;
                } else if (auto converted = fromUtf8(str); !converted) {
                    throw std::runtime_error(std::string{"Couldn't convert glyph \""} +  gl.str() + "\"");
                } else {
                    result.push_back(*converted);
                }
            }
        }

        return result;
    }
}
