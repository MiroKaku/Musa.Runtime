// Kernel-mode MBCS strspn -- C locale only.
//
// mbsspn.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strspn for byte span.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>
#include <string.h>

#ifndef _RETURN_PTR
extern "C" size_t __cdecl _mbsspn_l
#else  /* _RETURN_PTR */
extern "C" unsigned char * __cdecl _mbsspnp_l
#endif  /* _RETURN_PTR */
        (
        const unsigned char *string,
        const unsigned char *charset,
        _locale_t plocinfo
        )
{
        UNREFERENCED_PARAMETER(plocinfo);

        /* validation section */
#ifndef _RETURN_PTR
        _VALIDATE_RETURN(string != nullptr, EINVAL, 0);
        _VALIDATE_RETURN(charset != nullptr, EINVAL, 0);
#else  /* _RETURN_PTR */
        _VALIDATE_RETURN(string != nullptr, EINVAL, nullptr);
        _VALIDATE_RETURN(charset != nullptr, EINVAL, nullptr);
#endif  /* _RETURN_PTR */

#ifndef _RETURN_PTR
        return strspn((const char *)string, (const char *)charset);
#else  /* _RETURN_PTR */
        {
                size_t retval;
                retval = strspn((const char *)string, (const char *)charset);
                return (unsigned char *)(*(string + retval) ? string + retval : nullptr);
        }
#endif  /* _RETURN_PTR */
}

#ifndef _RETURN_PTR
extern "C" size_t (__cdecl _mbsspn)
#else  /* _RETURN_PTR */
extern "C" unsigned char * (__cdecl _mbsspnp)
#endif  /* _RETURN_PTR */
        (
        const unsigned char *string,
        const unsigned char *charset
        )
{
#ifndef _RETURN_PTR
        return _mbsspn_l(string, charset, nullptr);
#else  /* _RETURN_PTR */
        return _mbsspnp_l(string, charset, nullptr);
#endif  /* _RETURN_PTR */
}
