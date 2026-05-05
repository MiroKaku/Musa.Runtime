// Kernel-mode MBCS tok_s -- C locale only, delegates to strtok_s.
//
// mbstok_s.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Uses strtok_s.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>
#include <string.h>

extern "C" unsigned char * __cdecl _mbstok_s_l(
    unsigned char *_String,
    const unsigned char *_Control,
    unsigned char **_Context,
    _locale_t _Locale
    )
{
    UNREFERENCED_PARAMETER(_Locale);

    /* validation section */
    _VALIDATE_POINTER_ERROR_RETURN(_Context, EINVAL, nullptr);
    _VALIDATE_POINTER_ERROR_RETURN(_Control, EINVAL, nullptr);
    _VALIDATE_CONDITION_ERROR_RETURN(_String != nullptr || *_Context != nullptr, EINVAL, nullptr);

    return (unsigned char*)strtok_s((char *)_String, (const char *)_Control, (char **)_Context);
}

extern "C" unsigned char * __cdecl _mbstok_s(
    unsigned char *_String,
    const unsigned char *_Control,
    unsigned char **_Context
    )
{
    return _mbstok_s_l(_String, _Control, _Context, nullptr);
}
