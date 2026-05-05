// Kernel-mode MBCS substring search -- C locale only.
//
// mbsstr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strstr for byte search.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" _CONST_RETURN unsigned char * __cdecl _mbsstr_l(
        const unsigned char *str1,
        const unsigned char *str2,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(str2 != nullptr, EINVAL, nullptr);
        if (*str2 == '\0')
                return (unsigned char *)str1;
        _VALIDATE_RETURN(str1 != nullptr, EINVAL, nullptr);

        return (unsigned char *)strstr((const char *)str1, (const char *)str2);
}

extern "C" _CONST_RETURN unsigned char * (__cdecl _mbsstr)(
        const unsigned char *str1,
        const unsigned char *str2
        )
{
    return _mbsstr_l(str1, str2, nullptr);
}
