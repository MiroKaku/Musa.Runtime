// Kernel-mode MBCS character classification -- C locale only.
//
// ismbdgt.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII digits.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <ctype.h>

extern "C" int __cdecl _ismbcdigit_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);

    if (c <= 0x00FF)
    {
        return isdigit(c);
    }

    // Kernel mode: C locale, no MBCS characters beyond 0xFF
    return FALSE;
}

extern "C" int __cdecl _ismbcdigit(unsigned int const c)
{
    return _ismbcdigit_l(c, nullptr);
}
