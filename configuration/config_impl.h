#ifndef TTF2BPP_CONFIGURATION_H
#define TTF2BPP_CONFIGURATION_H
#include "ttf2bpp/configuration.h"
#include <yaml-cpp/yaml.h>
#include <concepts>
#include <algorithm>

namespace YAML {
    template<typename T>
    concept CFGOpt =
        std::same_as<T, ttf2bpp::Configuration::EncType> ||
        std::same_as<T, ttf2bpp::Configuration::WidthType>;
    template <CFGOpt T>
    struct convert<T> {
        static bool decode(const Node& node, T& rhs)
        {
            if (!node.IsScalar()) {
                return false;
            }
            auto val = node.as<std::string>();
            std::for_each(val.begin(), val.end(), ::tolower);
            if (val == "yaml" || val == "yml") {
                rhs = T::YAML;
            } else if (val == "64tass" || val == "asm") {
                rhs = T::TASS64;
            } else if constexpr (std::is_same_v<T, ttf2bpp::Configuration::WidthType>) {
                if (val == "csv") {
                    rhs = T::CSV;
                } else {
                    return false;
                }
            } else {
                return false;
            }
            return true;
        }
        static Node encode(const T& rhs)
        {
            Node n;
            if (rhs == T::YAML) {
                n = "yaml";
            } else if (rhs == T::TASS64) {
                n = "64tass";
            } else if constexpr (std::is_same_v<T, ttf2bpp::Configuration::WidthType>) {
                if (rhs == T::CSV) {
                    n = "csv";
                }
            }
            return n;
        }
    };

    template <>
    struct convert<ttf2bpp::Reserved> {
        static bool decode(const Node& node, ttf2bpp::Reserved& rhs);
    };
    template <>
    class convert<ttf2bpp::Configuration::Colors> {
    public:
        static bool decode(const Node& node, ttf2bpp::Configuration::Colors& rhs);
        static Node encode(const ttf2bpp::Configuration::Colors& rhs);
    };
    template <>
    struct convert<ttf2bpp::Configuration> {
        static bool decode(const Node& node, ttf2bpp::Configuration& rhs);
        static Node encode(const ttf2bpp::Configuration& rhs);
    };
    template <>
    struct convert<ttf2bpp::GlyphData> {
        static Node encode(const ttf2bpp::GlyphData& rhs);
    };
}

namespace ttf2bpp {

std::optional<unsigned long> fromUtf8(const std::string& utf8);
std::string toUtf8(unsigned long utf32);

} // namespace ttf2bpp

#endif // TTF2BPP_CONFIGURATION_H
