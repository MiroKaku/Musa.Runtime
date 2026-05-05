// Kernel-mode MBCS tok -- C locale only, delegates to strtok.
//
// mbstok.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Uses strtok.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal.h>
#include <corecrt_internal_mbstring.h>
#include <locale.h>
#include <stddef.h>
#include <string.h>

/***
* _mbstok_l - Break string into tokens (MBCS)
*
*Purpose:
*       Kernel mode: C locale, uses strtok with thread-local context.
*/

extern "C" unsigned char * __cdecl _mbstok_l(
    unsigned char*       const string,
    unsigned char const* const sepset,
    _locale_t            const locale
    )
{
    UNREFERENCED_PARAMETER(locale);
    return _mbstok_s_l(string, sepset, &__acrt_getptd()->_mbstok_token, nullptr);
}

extern "C" unsigned char * __cdecl _mbstok(
        unsigned char * string,
        const unsigned char * sepset
        )
{
    return _mbstok_l(string, sepset, nullptr);
}
