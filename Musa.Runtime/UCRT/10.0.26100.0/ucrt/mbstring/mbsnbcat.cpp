/***
*mbsnbcat.c - concatenate string2 onto string1, max length n bytes
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines mbsnbcat() - concatenate maximum of n bytes
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal.h>
#include <corecrt_internal_mbstring.h>
#include <string.h>

/***
* _mbsnbcat - concatenate max cnt bytes onto dst
*
*Purpose:
*       Concatenates src onto dst, with a maximum of cnt bytes copied.
*       Handles 2-byte MBCS characters correctly.
*
*******************************************************************************/

extern "C" unsigned char * __cdecl _mbsnbcat_l(
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
    return (unsigned char *)strncat((char *)dst, (const char *)src, cnt);
}

extern "C" unsigned char * (__cdecl _mbsnbcat)(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt
        )
{
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return _mbsnbcat_l(dst, src, cnt, nullptr);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}
