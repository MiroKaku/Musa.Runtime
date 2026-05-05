// Kernel-mode MBCS set_s -- C locale only, safe byte memset.
//
// mbsset_s.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Delegates to _strset_s.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>
#include <string.h>

extern "C" errno_t __cdecl _mbsset_s(
    unsigned char *_Dst,
    size_t _SizeInBytes,
    unsigned int _Value
    )
{
    /* validation section */
    _VALIDATE_STRING(_Dst, _SizeInBytes);

    return _strset_s((char *)_Dst, _SizeInBytes, (int)_Value);
}
