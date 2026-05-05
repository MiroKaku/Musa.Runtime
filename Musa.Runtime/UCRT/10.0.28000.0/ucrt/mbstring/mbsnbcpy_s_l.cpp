/***
*mbsnbcpy_s_l.c - Copy one string to another, n bytes only (MBCS)
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
#include <corecrt_internal_securecrt.h>

errno_t __cdecl _mbsnbcpy_s_l(
    unsigned char *_Dst,
    size_t _SizeInBytes,
    const unsigned char *_Src,
    size_t _CountInBytes,
    _LOCALE_ARG_DECL)
{
    UNREFERENCED_PARAMETER(_LOCALE_ARG);

    if (_CountInBytes == 0 && _Dst == nullptr && _SizeInBytes == 0)
    {
        _RETURN_NO_ERROR;
    }

    /* validation section */
    _VALIDATE_STRING(_Dst, _SizeInBytes);
    if (_CountInBytes == 0)
    {
        _RESET_STRING(_Dst, _SizeInBytes);
        _RETURN_NO_ERROR;
    }
    _VALIDATE_POINTER_RESET_STRING(_Src, _Dst, _SizeInBytes);

    /* C locale: use strncpy_s directly */
    return strncpy_s((char *)_Dst, _SizeInBytes, (const char *)_Src, _CountInBytes);
}
