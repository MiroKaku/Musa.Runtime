// Kernel-mode MBCS character search -- C locale only.
//
// mbschr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strchr for byte search.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

extern "C" _CONST_RETURN unsigned char * __cdecl _mbschr_l(
        const unsigned char *string,
        unsigned int c,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(string != nullptr, EINVAL, nullptr);

        return (_CONST_RETURN unsigned char *)strchr((const char *)string, (int)c);
}

extern "C" _CONST_RETURN unsigned char * (__cdecl _mbschr)(
        const unsigned char *string,
        unsigned int c
        )
{
    return _mbschr_l(string, c, nullptr);
}
