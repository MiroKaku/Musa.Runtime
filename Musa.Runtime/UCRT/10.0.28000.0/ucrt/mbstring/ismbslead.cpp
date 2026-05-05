// Kernel-mode MBCS context-sensitive lead byte check -- C locale only.
//
// ismbslead.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" int __cdecl _ismbslead_l(
    const unsigned char *string,
    const unsigned char *current,
    _locale_t plocinfo)
{
    UNREFERENCED_PARAMETER(plocinfo);

    /* validation section */
    _VALIDATE_RETURN(string != nullptr, EINVAL, 0);
    _VALIDATE_RETURN(current != nullptr, EINVAL, 0);

    // C locale: always return 0 (not a multibyte codepage)
    return 0;
}

extern "C" int __cdecl _ismbslead(
    const unsigned char *string,
    const unsigned char *current)
{
    return _ismbslead_l(string, current, nullptr);
}
