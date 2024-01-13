#include "ttf2bpp/glyphs.h"
#include "config_impl.h"
#include "iterator/iterator.h"
#include "utf_conv.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/putil.h>

namespace ttf2bpp {
    void setupDataDir(const std::string& path)
    {
        u_setDataDirectory(path.c_str());
    }
    std::optional<unsigned long> fromUtf8(const std::string& utf8)
    {
        UErrorCode err = U_ZERO_ERROR;
        auto utf16Str = icu::UnicodeString::fromUTF8(utf8);

        if (auto instance = icu::Normalizer2::getNFCInstance(err); U_FAILURE(err)) {
            throw std::runtime_error("couldn't get the NFC instance.");
        } else if (instance->normalize(utf16Str, err) ; U_FAILURE(err)) {
            throw std::runtime_error("cound't perform NFC normalization.");
        }

        return (unsigned long)utf16Str.char32At(0);
    }

    std::string toUtf8(unsigned long utf32)
    {
        std::string ret{""};
        icu::UnicodeString::fromUTF32((int32_t*)&utf32, 1).toUTF8String(ret);
        return ret;
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
        auto lc = icu::Locale::createCanonical("en_US.utf-8");
        while (std::getline(inFile, line)) {
            for (adapter::BreakIterator bi(false, line, lc); !bi.done(); ++bi) {
                // TODO: See if there's non case-by-case way to handle formatting glyphs.
                if (auto str = *bi; (str == "\t" ||  str == "\n" || str== "\r" || str == "\u00A0")) {
                    continue;
                } else if (auto converted = fromUtf8(str); !converted) {
                    throw std::runtime_error(std::string{"Couldn't convert glyph \""} +  *bi + "\"");
                } else {
                    result.push_back(*converted);
                }
            }
        }

        return result;
    }
}
