#ifndef TTF2BPP_UNICODEDATABASE_H
#define TTF2BPP_UNICODEDATABASE_H

#include <string>
#include <vector>

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    #ifdef TTF_UNICODE_DLL
        #if defined(__GNUC__)
            #define TTF_UNICODE_EXPORT __attribute__ ((dllexport))
        #else
            #define TTF_UNICODE_EXPORT __declspec(dllexport)
        #endif
    #else
        #if defined(__GNUC__)
            #define TTF_UNICODE_EXPORT __attribute__ ((dllimport))
        #else
            #define TTF_UNICODE_EXPORT __declspec(dllimport)
        #endif
    #endif
#else
    #ifdef TTF_UNICODE_DLL
        #define TTF_UNICODE_EXPORT __attribute__ ((visibility ("default")))
    #else
        #define TTF_UNICODE_EXPORT
    #endif
#endif


namespace ttf2bpp {
// not thread safe
struct TTF_UNICODE_EXPORT UnicodeDatabase
{
    std::vector<unsigned long> codes;
    static UnicodeDatabase getUnicodeDatabase(const std::string& file, std::vector<std::string>&& blocks);
};

} // namespace ttf2bpp

#endif // TTF2BPP_UNICODEDATABASE_H
