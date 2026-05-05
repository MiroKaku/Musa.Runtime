// Kernel-mode MBCS character length -- C locale (CP_UTF8).
//
// mbclen.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to CP_UTF8.
// UTF-8 sequence lengths:
//   0xxxxxxx     → 1 byte
//   110xxxxx     → 2 bytes
//   1110xxxx     → 3 bytes
//   11110xxx     → 4 bytes

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" size_t __cdecl _mbclen_l(unsigned char const* c, _locale_t locale)
{
    UNREFERENCED_PARAMETER(locale);

    if (c == nullptr)
        return 1;

    unsigned char const b = *c;
    if (b < 0x80)       return 1;  // 0xxxxxxx
    if (b < 0xE0)       return 2;  // 110xxxxx
    if (b < 0xF0)       return 3;  // 1110xxxx
    if (b < 0xF8)       return 4;  // 11110xxx
    return 1;  // Invalid lead byte, default to 1
}

extern "C" size_t __cdecl _mbclen(unsigned char const* c)
{
    return _mbclen_l(c, nullptr);
}
