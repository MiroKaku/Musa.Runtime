// Kernel-mode MBCS byte collation -- C locale only.
//
// mbsnbcol.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Uses strncmp.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <limits.h>
#include <string.h>

extern "C" int __cdecl _mbsnbcoll_l(
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
    _VALIDATE_RETURN(n <= INT_MAX, EINVAL, _NLSCMPERROR);

    return strncmp((const char *)s1, (const char *)s2, n);
}

extern "C" int (__cdecl _mbsnbcoll)(
        const unsigned char *s1,
        const unsigned char *s2,
        size_t n
        )
{
    return _mbsnbcoll_l(s1, s2, n, nullptr);
}
