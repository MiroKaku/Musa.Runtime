/***
*mbsnbset_s_l.c - Sets first n bytes of string to given character (MBCS)
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
#include <corecrt_internal_securecrt.h>

errno_t __cdecl _mbsnbset_s_l(
    unsigned char *_Dst,
    size_t _SizeInBytes,
    unsigned int _Value,
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

    /* C locale: use _strnset_s directly */
    return _strnset_s((char *)_Dst, _SizeInBytes, (int)_Value, _CountInBytes);
}
