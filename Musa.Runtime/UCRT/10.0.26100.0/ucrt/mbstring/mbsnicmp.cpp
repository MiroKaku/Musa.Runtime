// Kernel-mode MBCS character case-insensitive comparison -- C locale only.
//
// mbsnicmp.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Uses strnicmp.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" int __cdecl _mbsnicmp_l(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (n == 0)
        return 0;

    /* validation section */
    _VALIDATE_RETURN(s1 != nullptr, EINVAL, _NLSCMPERROR);
    _VALIDATE_RETURN(s2 != nullptr, EINVAL, _NLSCMPERROR);

    return _strnicmp((const char *)s1, (const char *)s2, n);
}

extern "C" int (__cdecl _mbsnicmp)(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
    return _mbsnicmp_l(s1, s2, n, nullptr);
}
