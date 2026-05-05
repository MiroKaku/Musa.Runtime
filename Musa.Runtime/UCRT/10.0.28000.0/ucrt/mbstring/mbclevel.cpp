// Kernel-mode MBCS Kanji level classification -- C locale only.
//
// mbclevel.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. No Kanji code page exists.
// All Kanji level classification functions return FALSE.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

/***
*int _ismbcl0_l(c, locale) - Tests if char is hiragana, katakana, alphabet or digit.
*
*Purpose:
*       Kernel mode: C locale, no Kanji code page. Always returns FALSE.
*/

extern "C" int __cdecl _ismbcl0_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(plocinfo);
    return FALSE;
}

extern "C" int (__cdecl _ismbcl0)(
        unsigned int c
        )
{
    return _ismbcl0_l(c, nullptr);
}


/***
*int _ismbcl1_l(c, locale) - Tests for 1st-level Microsoft Kanji code set.
*
*Purpose:
*       Kernel mode: C locale, no Kanji code page. Always returns FALSE.
*/

extern "C" int __cdecl _ismbcl1_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(plocinfo);
    return FALSE;
}

extern "C" int (__cdecl _ismbcl1)(
    unsigned int c
    )
{
    return _ismbcl1_l(c, nullptr);
}


/***
*int _ismbcl2_l(c, locale) - Tests for a 2nd-level Microsoft Kanji code character.
*
*Purpose:
*       Kernel mode: C locale, no Kanji code page. Always returns FALSE.
*/

extern "C" int __cdecl _ismbcl2_l(
        unsigned int c,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(plocinfo);
    return FALSE;
}

extern "C" int __cdecl _ismbcl2(
        unsigned int c
        )
{
    return _ismbcl2_l(c, nullptr);
}
