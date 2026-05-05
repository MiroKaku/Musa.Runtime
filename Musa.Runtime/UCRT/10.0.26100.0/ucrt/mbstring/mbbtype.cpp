// Kernel-mode MBCS byte type -- C locale (CP_UTF8).
//
// mbbtype.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to CP_UTF8.
// UTF-8 lead byte: 11xxxxxx (0xC0-0xFF), trail byte: 10xxxxxx (0x80-0xBF)

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" int __cdecl _mbbtype_l(
    unsigned char const c,
    int           const ctype,
    _locale_t     const locale
    )
{
    UNREFERENCED_PARAMETER(ctype);
    UNREFERENCED_PARAMETER(locale);

    // UTF-8 classification
    if (c < 0x80)
        return _MBC_SINGLE;    // 0xxxxxxx: single byte
    if (c >= 0xC0)
        return _MBC_LEAD;      // 11xxxxxx: lead byte
    return _MBC_TRAIL;         // 10xxxxxx: trail byte
}

extern "C" int __cdecl _mbbtype(unsigned char const c, int const ctype)
{
    return _mbbtype_l(c, ctype, nullptr);
}
