// Kernel-mode MBCS nset -- C locale only, byte memset.
//
// mbsnset.cpp
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
* _mbsnset_l - Sets first n characters of string to given character (MBCS)
*
*Purpose:
*       Kernel mode: C locale, simple byte memset.
*/

extern "C" unsigned char * __cdecl _mbsnset_l(
        unsigned char *string,
        unsigned int val,
        size_t count,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    /* validation section */
    _VALIDATE_RETURN(string != nullptr || count == 0, EINVAL, nullptr);

    return (unsigned char *)_strnset((char *)string, val, count);
}

extern "C" unsigned char * (__cdecl _mbsnset)(
        unsigned char *string,
        unsigned int val,
        size_t count
        )
{
    return _mbsnset_l(string, val, count, nullptr);
}
