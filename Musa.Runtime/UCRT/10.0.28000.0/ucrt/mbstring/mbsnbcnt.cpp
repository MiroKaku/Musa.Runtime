// Kernel-mode MBCS byte count -- C locale only.
//
// mbsnbcnt.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Each char = 1 byte.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

extern "C" size_t __cdecl _mbsnbcnt_l(
    const unsigned char *string,
    size_t ccnt,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (string == nullptr && ccnt != 0)
        return 0;

    // Kernel mode: CP_UTF8, each character = 1 byte
    size_t len = strnlen((const char *)string, ccnt);
    return len < ccnt ? len : ccnt;
}

extern "C" size_t (__cdecl _mbsnbcnt)(
    const unsigned char *string,
    size_t ccnt
    )
{
    return _mbsnbcnt_l(string, ccnt, nullptr);
}
