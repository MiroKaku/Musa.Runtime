// Kernel-mode MBCS character lowercase -- C locale, ASCII only.
//
// mbtolwr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII tolower.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <ctype.h>

extern "C" unsigned int __cdecl _mbctolower_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (c > 0x00FF)
        return c;

    return (unsigned int)tolower((int)c);
}

extern "C" unsigned int (__cdecl _mbctolower)(
        unsigned int c
        )
{
    return _mbctolower_l(c, nullptr);
}
