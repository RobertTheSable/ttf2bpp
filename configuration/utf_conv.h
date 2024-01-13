#ifndef UTF_CONV_H
#define UTF_CONV_H

#include <optional>
#include <string>

namespace ttf2bpp {

std::optional<unsigned long> fromUtf8(const std::string& utf8);
std::string toUtf8(unsigned long utf32);

}

#endif // UTF_CONV_H
