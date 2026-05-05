// Kernel-mode MBCS increment -- C locale only.
//
// mbsinc.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Simple byte increment.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal.h>
#include <corecrt_internal_mbstring.h>
#include <stddef.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

extern "C" unsigned char * __cdecl _mbsinc_l(
        const unsigned char *current,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        return (unsigned char *)(current + 1);
}

extern "C" unsigned char * (__cdecl _mbsinc)(
        const unsigned char *current
        )
{
        /* validation section */
        _VALIDATE_RETURN(current != nullptr, EINVAL, nullptr);

        return (unsigned char *)(current + 1);
}
