// Kernel-mode MBCS character classification -- C locale only.
//
// ismbalph.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII alphabetic.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <ctype.h>

extern "C" int __cdecl _ismbcalpha_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);

    if (c <= 0x00FF)
    {
        return _ismbbalpha(c);
    }

    // Kernel mode: C locale, no MBCS characters beyond 0xFF
    return FALSE;
}

extern "C" int __cdecl _ismbcalpha(unsigned int const c)
{
    return _ismbcalpha_l(c, nullptr);
}
