// Kernel-mode MBCS next character -- C locale only.
//
// mbsnextc.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Simple byte read.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <stdlib.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

extern "C" unsigned int __cdecl _mbsnextc_l(
        const unsigned char *s,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(s != nullptr, EINVAL, 0);

        return (unsigned int)*s;
}

extern "C" unsigned int (__cdecl _mbsnextc)(
        const unsigned char *s
        )
{
    return _mbsnextc_l(s, nullptr);
}
