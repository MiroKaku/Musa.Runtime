/***
*mbscat_s_l.c - Concatenate one string to another (MBCS)
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Concatenate one string to another (MBCS)
*
*******************************************************************************/
#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>

errno_t __cdecl _mbscat_s_l(unsigned char *_Dst, size_t _SizeInBytes, const unsigned char *_Src, _LOCALE_ARG_DECL)
{
    UNREFERENCED_PARAMETER(_LOCALE_ARG);

    /* validation section */
    _VALIDATE_STRING(_Dst, _SizeInBytes);
    _VALIDATE_POINTER_RESET_STRING(_Src, _Dst, _SizeInBytes);

    /* C locale: use strcat_s directly */
    return strcat_s((char *)_Dst, _SizeInBytes, (const char *)_Src);
}
