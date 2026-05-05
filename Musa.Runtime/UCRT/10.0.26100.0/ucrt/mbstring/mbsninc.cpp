// Kernel-mode MBCS n-increment -- C locale only.
//
// mbsninc.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Simple byte skip.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <stddef.h>

extern "C" unsigned char * __cdecl _mbsninc_l(
        const unsigned char *string,
        size_t ccnt,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        if (string == nullptr)
                return nullptr;

        return const_cast<unsigned char*>(string) + ccnt;
}

extern "C" unsigned char * (__cdecl _mbsninc)(
        const unsigned char *string,
        size_t ccnt
        )
{
    return _mbsninc_l(string, ccnt, nullptr);
}
