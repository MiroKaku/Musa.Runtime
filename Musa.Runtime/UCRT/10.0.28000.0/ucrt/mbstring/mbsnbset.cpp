/***
*mbsnbset.c - Sets first n bytes of string to given character (MBCS)
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Sets first n bytes of string to given character (MBCS)
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

/***
* _mbsnbset - Sets first n bytes of string to given character (MBCS)
*
*Purpose:
*       Sets the first n bytes of string to the supplied
*       character value.
*
*******************************************************************************/

extern "C" unsigned char * __cdecl _mbsnbset_l(
        unsigned char *string,
        unsigned int val,
        size_t count,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    /* validation section */
    _VALIDATE_RETURN(string != nullptr || count == 0, EINVAL, nullptr);

    /* C locale: use _strnset directly */
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return (unsigned char *)_strnset((char *)string, val, count);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}

extern "C" unsigned char * (__cdecl _mbsnbset)(
        unsigned char *string,
        unsigned int val,
        size_t count
        )
{
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return _mbsnbset_l(string, val, count, nullptr);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}
