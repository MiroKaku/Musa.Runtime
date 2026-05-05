/***
*mbsnccnt.c - Return char count of MBCS string
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Return char count of MBCS string
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

/***
* _mbsnccnt - Return char count of MBCS string
*
*Purpose:
*       Returns the number of chars between the start of the supplied
*       string and the byte count supplied.
*
*******************************************************************************/

extern "C" size_t __cdecl _mbsnccnt_l(
        const unsigned char *string,
        size_t bcnt,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    _VALIDATE_RETURN(string != nullptr || bcnt == 0, EINVAL, 0);

    /* C locale: every byte is one character */
    size_t n = 0;
    while (bcnt-- && *string)
    {
        n++;
        string++;
    }

    return n;
}

extern "C" size_t (__cdecl _mbsnccnt)(
        const unsigned char *string,
        size_t bcnt
        )
{
    return _mbsnccnt_l(string, bcnt, nullptr);
}
