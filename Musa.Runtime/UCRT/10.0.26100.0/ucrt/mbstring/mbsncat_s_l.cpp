// Kernel-mode MBCS string concatenate -- C locale only.
//
// mbsncat_s_l.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Delegates to strncat_s.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>
#include <string.h>

extern "C" errno_t __cdecl _mbsncat_s_l(
    unsigned char *dst,
    size_t sizeInBytes,
    const unsigned char *src,
    size_t count,
    _locale_t locale
    )
{
    UNREFERENCED_PARAMETER(locale);

    if (dst == nullptr || src == nullptr)
        return EINVAL;

    // Kernel mode: CP_UTF8, each character = 1 byte
    size_t src_len = strnlen((const char *)src, count);
    return strncat_s((char *)dst, sizeInBytes, (const char *)src, src_len);
}
