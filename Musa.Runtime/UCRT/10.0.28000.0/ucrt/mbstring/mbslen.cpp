// Kernel-mode MBCS string length -- C locale only.
//
// mbslen.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strlen for byte length.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" size_t __cdecl _mbslen_l(
        const unsigned char *s,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        return strlen((const char *)s);
}

extern "C" size_t (__cdecl _mbslen)(
        const unsigned char *s
        )
{
    return _mbslen_l(s, nullptr);
}
