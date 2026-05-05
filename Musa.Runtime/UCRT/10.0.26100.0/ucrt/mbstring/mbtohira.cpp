// Kernel-mode MBCS katakana to hiragana -- C locale, no-op.
//
// mbtohira.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. No conversion, return input.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" unsigned int __cdecl _mbctohira_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);
    return c;
}

extern "C" unsigned int __cdecl _mbctohira(unsigned int c)
{
    return _mbctohira_l(c, nullptr);
}
