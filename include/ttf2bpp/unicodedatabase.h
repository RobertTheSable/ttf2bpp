#ifndef TTF2BPP_UNICODEDATABASE_H
#define TTF2BPP_UNICODEDATABASE_H

#include <string>
#include <vector>

namespace ttf2bpp {
// not thread safe
struct UnicodeDatabase
{
    std::string block, category;
    std::vector<unsigned long> codes;
    static UnicodeDatabase getUnicodeDatabase(const std::string& file, const std::string &block, const std::string &category);
};

} // namespace ttf2bpp

#endif // TTF2BPP_UNICODEDATABASE_H
