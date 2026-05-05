/***
*mbsnbcpy.c - Copy one string to another, n bytes only (MBCS)
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Copy one string to another, n bytes only (MBCS)
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

/***
* _mbsnbcpy - Copy one string to another, n bytes only (MBCS)
*
*Purpose:
*       Copies exactly cnt bytes from src to dst.  If strlen(src) < cnt, the
*       remaining character are padded with null bytes.  If strlen >= cnt, no
*       terminating null byte is added.  2-byte MBCS characters are handled
*       correctly.
*
*******************************************************************************/

#pragma warning(suppress:6101)
unsigned char * __cdecl _mbsnbcpy_l(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    unsigned char *start = dst;

    /* validation section */
    _VALIDATE_RETURN(dst != nullptr || cnt == 0, EINVAL, nullptr);
    _VALIDATE_RETURN(src != nullptr || cnt == 0, EINVAL, nullptr);

    /* C locale: use strncpy directly */
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return (unsigned char *)strncpy((char *)dst, (const char *)src, cnt);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}

unsigned char * (__cdecl _mbsnbcpy)(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt
        )
{
    _BEGIN_SECURE_CRT_DEPRECATION_DISABLE
    return _mbsnbcpy_l(dst, src, cnt, nullptr);
    _END_SECURE_CRT_DEPRECATION_DISABLE
}
