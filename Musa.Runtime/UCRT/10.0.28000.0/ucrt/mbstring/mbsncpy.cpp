// Kernel-mode MBCS string copy -- C locale only.
//
// mbsncpy.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Delegates to strncpy.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#pragma warning(disable:__WARNING_INCORRECT_VALIDATION __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED)

extern "C" unsigned char * __cdecl _mbsncpy_l(
    unsigned char *dst,
    const unsigned char *src,
    size_t cnt,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (dst == nullptr && cnt != 0)
        return nullptr;
    if (src == nullptr && cnt != 0)
        return nullptr;

    // Kernel mode: CP_UTF8, each character = 1 byte
    return (unsigned char *)strncpy((char *)dst, (const char *)src, cnt);
}

extern "C" unsigned char * (__cdecl _mbsncpy)(
    unsigned char *dst,
    const unsigned char *src,
    size_t cnt
    )
{
    return _mbsncpy_l(dst, src, cnt, nullptr);
}
