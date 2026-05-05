// Kernel-mode MBCS string collation -- C locale only.
//
// mbscoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Uses strcmp.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" int __cdecl _mbscoll_l(
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

extern "C" int (__cdecl _mbscoll)(
        const unsigned char *s1,
        const unsigned char *s2
        )
{
    return _mbscoll_l(s1, s2, nullptr);
}
