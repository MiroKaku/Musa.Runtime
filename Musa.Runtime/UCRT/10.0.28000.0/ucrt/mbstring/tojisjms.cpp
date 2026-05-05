// Kernel-mode JIS/JMS conversion -- C locale, no-op.
//
// tojisjms.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. No conversion, return input.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" unsigned int __cdecl _mbcjistojms_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);
    return c;
}

extern "C" unsigned int (__cdecl _mbcjistojms)(
        unsigned int c
        )
{
    return _mbcjistojms_l(c, nullptr);
}

extern "C" unsigned int __cdecl _mbcjmstojis_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);
    return c;
}

extern "C" unsigned int (__cdecl _mbcjmstojis)(
        unsigned int c
        )
{
    return _mbcjmstojis_l(c, nullptr);
}
