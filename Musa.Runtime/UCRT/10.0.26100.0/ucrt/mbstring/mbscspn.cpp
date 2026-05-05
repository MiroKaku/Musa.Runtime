// Kernel-mode MBCS strcspn -- C locale only.
//
// mbscspn.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strcspn/strpbrk for byte operations.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#ifndef _RETURN_PTR

extern "C" size_t __cdecl _mbscspn_l(
        const unsigned char *string,
        const unsigned char *charset,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(string != nullptr, EINVAL, 0);
        _VALIDATE_RETURN(charset != nullptr, EINVAL, 0);

        return strcspn((const char *)string, (const char *)charset);
}

extern "C" size_t (__cdecl _mbscspn)(
        const unsigned char *string,
        const unsigned char *charset
        )
{
        return _mbscspn_l(string, charset, nullptr);
}

#else  /* _RETURN_PTR */

extern "C" const unsigned char * __cdecl _mbspbrk_l(
        const unsigned char *string,
        const unsigned char  *charset,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
        _VALIDATE_RETURN(string != nullptr, EINVAL, nullptr);
        _VALIDATE_RETURN(charset != nullptr, EINVAL, nullptr);

        return (const unsigned char *)strpbrk((const char *)string, (const char *)charset);
}

extern "C" const unsigned char * (__cdecl _mbspbrk)(
        const unsigned char *string,
        const unsigned char  *charset
        )
{
        return _mbspbrk_l(string, charset, nullptr);
}

#endif  /* _RETURN_PTR */
