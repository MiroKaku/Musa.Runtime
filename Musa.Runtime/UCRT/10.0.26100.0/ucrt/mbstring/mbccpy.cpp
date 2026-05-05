// Kernel-mode MBCS character copy -- C locale (CP_UTF8).
//
// mbccpy.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to CP_UTF8.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

// Undefine macros to avoid conflicts with function definitions
#undef _ismbblead_l
#undef _ismbblead

#include <corecrt_internal_mbstring.h>

static int __cdecl utf8_char_len(unsigned char b)
{
    if (b < 0x80)  return 1;
    if (b < 0xE0)  return 2;
    if (b < 0xF0)  return 3;
    if (b < 0xF8)  return 4;
    return 1;
}

extern "C" void __cdecl _mbccpy_l(
    unsigned char *dst,
    const unsigned char *src,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (dst == nullptr || src == nullptr)
        return;

    int len = utf8_char_len(*src);

    // Copy up to 4 bytes
    for (int i = 0; i < len; ++i)
        dst[i] = src[i];
}

extern "C" void (__cdecl _mbccpy)(
    unsigned char *dst,
    const unsigned char *src
    )
{
    _mbccpy_l(dst, src, nullptr);
}
