// Kernel-mode MBCS reverse char search -- C locale only.
//
// mbsrchr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strrchr for byte search.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

extern "C" _CONST_RETURN unsigned char * __cdecl _mbsrchr_l(
        const unsigned char *str,
        unsigned int c,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(str != nullptr, EINVAL, nullptr);

        return (_CONST_RETURN unsigned char *)strrchr((const char *)str, (int)c);
}

extern "C" _CONST_RETURN unsigned char * (__cdecl _mbsrchr)(
        const unsigned char *str,
        unsigned int c
        )
{
    return _mbsrchr_l(str, c, nullptr);
}
