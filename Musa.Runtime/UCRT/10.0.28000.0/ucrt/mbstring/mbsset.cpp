// Kernel-mode MBCS set -- C locale only, byte memset.
//
// mbsset.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Simple byte memset.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <locale.h>
#include <string.h>


/***
* _mbsset_l - Sets all characters of string to given character (MBCS)
*
*Purpose:
*       Kernel mode: C locale, simple byte memset.
*/

extern "C" unsigned char * __cdecl _mbsset_l(
        unsigned char *string,
        unsigned int val,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    /* validation section */
    _VALIDATE_RETURN(string != nullptr, EINVAL, nullptr);

    return (unsigned char *)_strset((char *)string, val);
}

extern "C" unsigned char * (__cdecl _mbsset)(
        unsigned char *string,
        unsigned int val
        )
{
    return _mbsset_l(string, val, nullptr);
}
