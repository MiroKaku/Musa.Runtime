// Kernel-mode MBCS character uppercase -- C locale, ASCII only.
//
// mbtoupr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII toupper.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <ctype.h>

extern "C" unsigned int __cdecl _mbctoupper_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (c > 0x00FF)
        return c;

    return (unsigned int)toupper((int)c);
}

extern "C" unsigned int (__cdecl _mbctoupper)(
        unsigned int c
        )
{
    return _mbctoupper_l(c, nullptr);
}
