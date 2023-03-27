#ifndef TTF2BPP_CONFIGURATION_H
#define TTF2BPP_CONFIGURATION_H
#include "ttf2bpp/configuration.h"
#include <yaml-cpp/yaml.h>

namespace YAML {
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


class configuration
{
public:
    configuration();
};

} // namespace ttf2bpp

#endif // TTF2BPP_CONFIGURATION_H
