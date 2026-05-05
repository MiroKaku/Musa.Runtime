/***
*mbsncat.c - concatenate string2 onto string1, max length n
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines mbsncat() - concatenate maximum of n characters
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

/***
* _mbsncat - concatenate max cnt characters onto dst
*
*Purpose:
*       Concatenates src onto dst, with a maximum of cnt characters copied.
*       Handles 2-byte MBCS characters correctly.
*
*******************************************************************************/

extern "C" unsigned char * __cdecl _mbsncat_l(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (!cnt)
        return(dst);

    /* validation section */
    _VALIDATE_RETURN(dst != nullptr, EINVAL, nullptr);
    _VALIDATE_RETURN(src != nullptr, EINVAL, nullptr);

    /* C locale: use strncat directly */
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return (unsigned char *)strncat((char *)dst, (const char *)src, cnt);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}

extern "C" unsigned char * (__cdecl _mbsncat)(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt
        )
{
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return _mbsncat_l(dst, src, cnt, nullptr);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}
