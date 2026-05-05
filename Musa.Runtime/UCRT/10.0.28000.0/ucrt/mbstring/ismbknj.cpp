// Kernel-mode MBCS Kanji character classification -- C locale only.
//
// ismbknj.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. No Kanji support; all return FALSE.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" int __cdecl _ismbchira_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(locale);
    // Kernel mode: C locale, no Kanji codepage
    return FALSE;
}

extern "C" int __cdecl _ismbchira(unsigned int const c)
{
    return _ismbchira_l(c, nullptr);
}

extern "C" int __cdecl _ismbckata_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(locale);
    // Kernel mode: C locale, no Kanji codepage
    return FALSE;
}

extern "C" int __cdecl _ismbckata(unsigned int const c)
{
    return _ismbckata_l(c, nullptr);
}

extern "C" int __cdecl _ismbcsymbol_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(c);
    UNREFERENCED_PARAMETER(locale);
    // Kernel mode: C locale, no Kanji codepage; no ASCII symbol equivalent
    return FALSE;
}

extern "C" int __cdecl _ismbcsymbol(unsigned int const c)
{
    return _ismbcsymbol_l(c, nullptr);
}
