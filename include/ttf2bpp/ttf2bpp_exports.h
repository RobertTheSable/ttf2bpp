#ifndef TTF2BPP_EXPORTS_H
#define TTF2BPP_EXPORTS_H

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    #ifdef TTF_BPP_DLL
        #if defined(__GNUC__)
            #define TTF_BPP_EXPORT __attribute__ ((dllexport))
        #else
            #define TTF_BPP_EXPORT __declspec(dllexport)
        #endif
    #else
        #if defined(__GNUC__)
            #define TTF_BPP_EXPORT __attribute__ ((dllimport))
        #else
            #define TTF_BPP_EXPORT __declspec(dllimport)
        #endif
    #endif
#else
    #ifdef TTF_BPP_DLL
        #define TTF_BPP_EXPORT __attribute__ ((visibility ("default")))
    #else
        #define TTF_BPP_EXPORT
    #endif
#endif

#endif // TTF2BPP_EXPORTS_H
