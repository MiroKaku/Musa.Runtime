// Kernel-mode MBCS string comparison -- C locale only.
//
// mbscmp.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strcmp for byte comparison.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

extern "C" int __cdecl _mbscmp_l(
        const unsigned char *s1,
        const unsigned char *s2,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(s1 != nullptr, EINVAL, _NLSCMPERROR);
        _VALIDATE_RETURN(s2 != nullptr, EINVAL, _NLSCMPERROR);

        return strcmp((const char *)s1, (const char *)s2);
}

extern "C" int (__cdecl _mbscmp)(
        const unsigned char *s1,
        const unsigned char *s2
        )
{
    return _mbscmp_l(s1, s2, nullptr);
}
