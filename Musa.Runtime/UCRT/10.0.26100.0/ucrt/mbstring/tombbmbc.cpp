// Kernel-mode half-width/full-width conversion -- C locale, no-op.
//
// tombbmbc.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. No conversion, return input.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" unsigned int __cdecl _mbbtombc_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);
    return c;
}

extern "C" unsigned int (__cdecl _mbbtombc)(
        unsigned int c
        )
{
    return _mbbtombc_l(c, nullptr);
}

extern "C" unsigned int __cdecl _mbctombb_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);
    return c;
}

extern "C" unsigned int (__cdecl _mbctombb)(
        unsigned int c
        )
{
    return _mbctombb_l(c, nullptr);
}
