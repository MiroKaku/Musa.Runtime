// Kernel-mode MBCS nset_s -- C locale only, safe byte memset.
//
// mbsnset_s.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Delegates to _strnset_s.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>
#include <string.h>

extern "C" errno_t __cdecl _mbsnset_s(
    unsigned char *_Dst,
    size_t _SizeInBytes,
    unsigned int _Value,
    size_t _CountInChars
    )
{
    /* validation section */
    if (_CountInChars == 0 && _Dst == nullptr && _SizeInBytes == 0)
    {
        return 0;
    }
    _VALIDATE_STRING(_Dst, _SizeInBytes);

    return _strnset_s((char *)_Dst, _SizeInBytes, (int)_Value, _CountInChars);
}
