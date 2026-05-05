// Kernel-mode MBCS string reverse -- C locale only.
//
// mbsrev.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. _strrev for byte reversal.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" unsigned char * __cdecl _mbsrev_l(
        unsigned char *string,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(string != nullptr, EINVAL, nullptr);

        return (unsigned char *)_strrev((char *)string);
}

extern "C" unsigned char * (__cdecl _mbsrev)(
        unsigned char *string
        )
{
    return _mbsrev_l(string, nullptr);
}
