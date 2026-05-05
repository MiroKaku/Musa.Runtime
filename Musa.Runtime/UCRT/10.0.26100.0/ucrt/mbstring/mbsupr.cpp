// Kernel-mode MBCS string uppercase -- C locale, ASCII only.
//
// mbsupr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII toupper.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_securecrt.h>
#include <ctype.h>
#include <string.h>

extern "C" errno_t __cdecl _mbsupr_s_l(
        unsigned char *string,
        size_t sizeInBytes,
        _locale_t plocinfo
        )
{
    UNREFERENCED_PARAMETER(plocinfo);

    /* validation section */
    _VALIDATE_RETURN_ERRCODE((string != nullptr && sizeInBytes > 0) || (string == nullptr && sizeInBytes == 0), EINVAL);

    if (string == nullptr)
    {
        return 0;
    }

    size_t stringlen = strnlen((char *)string, sizeInBytes);
    if (stringlen >= sizeInBytes)
    {
        _RESET_STRING(string, sizeInBytes);
        _RETURN_DEST_NOT_NULL_TERMINATED(string, sizeInBytes);
    }
    _FILL_STRING(string, sizeInBytes, stringlen + 1);

    for (unsigned char *cp = string; *cp != '\0'; ++cp)
    {
        if (*cp <= 0xFF)
        {
            *cp = (unsigned char)toupper(*cp);
        }
    }

    return 0;
}

extern "C" errno_t (__cdecl _mbsupr_s)(
        unsigned char *string,
        size_t sizeInBytes
        )
{
    return _mbsupr_s_l(string, sizeInBytes, nullptr);
}

extern "C" unsigned char * (__cdecl _mbsupr_l)(
        unsigned char *string,
        _locale_t plocinfo
        )
{
    return (_mbsupr_s_l(string, (string == nullptr ? 0 : (size_t)-1), plocinfo) == 0 ? string : nullptr);
}

extern "C" unsigned char * (__cdecl _mbsupr)(
        unsigned char *string
        )
{
    return (_mbsupr_s_l(string, (string == nullptr ? 0 : (size_t)-1), nullptr) == 0 ? string : nullptr);
}
