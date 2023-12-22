#ifndef TRIFECTA_UCHAR_H
#define TRIFECTA_UCHAR_H

/* Apple does not provide uchar.h */
#ifdef __APPLE__
    #include <stdint.h>
    typedef uint_least16_t char16_t;
    typedef uint_least32_t char32_t;
#else
    #include <uchar.h>
#endif

#endif