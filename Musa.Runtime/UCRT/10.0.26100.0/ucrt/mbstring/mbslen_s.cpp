// Kernel-mode MBCS bounded string length -- C locale only.
//
// mbslen_s.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strnlen for bounded byte length.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

size_t __cdecl _mbsnlen_l(
        const unsigned char *s,
        size_t sizeInBytes,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        return strnlen((const char *)s, sizeInBytes);
}

size_t __cdecl _mbsnlen(
        const unsigned char *s,
        size_t maxsize
        )
{
    return _mbsnlen_l(s, maxsize, nullptr);
}
