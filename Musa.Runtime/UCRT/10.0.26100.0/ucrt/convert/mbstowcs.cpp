// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// mbstowcs.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale with CP_UTF8 codepage.
// All locale-dependent branches are eliminated.

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>
#include <corecrt_internal_securecrt.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>

using namespace __crt_mbstring;

/* Helper shared by secure and non-secure functions */

static size_t __cdecl _mbstowcs_l_helper(
    _Out_writes_opt_z_(n)               wchar_t *              pwcs,
    _In_reads_or_z_(n) _Pre_z_          const char *           s,
    _In_                                size_t                 n,
    _In_opt_                            __crt_cached_ptd_host& ptd
    ) throw()
{
    UNREFERENCED_PARAMETER(ptd);

    size_t count = 0;

    if (pwcs && n == 0)
    {
        return (size_t) 0;
    }

    if (pwcs && n > 0)
    {
        *pwcs = L'\0';
    }

    if (s == nullptr)
    {
        return (size_t)-1;
    }

    // Kernel mode: always CP_UTF8
    mbstate_t state{};
    return __mbsrtowcs_utf8(pwcs, &s, n, &state, ptd);
}

extern "C" size_t __cdecl _mbstowcs_l(
    wchar_t  *pwcs,
    const char *s,
    size_t n,
    _locale_t plocinfo
    )
{
    __crt_cached_ptd_host ptd(plocinfo);
    return _mbstowcs_l_helper(pwcs, s, n, ptd);
}

extern "C" size_t __cdecl mbstowcs(
    wchar_t  *pwcs,
    const char *s,
    size_t n
    )
{
    __crt_cached_ptd_host ptd;
    return _mbstowcs_l_helper(pwcs, s, n, ptd);
}

static errno_t __cdecl _mbstowcs_internal(
    size_t *               pConvertedChars,
    wchar_t *              pwcs,
    size_t                 sizeInWords,
    const char *           s,
    size_t                 n,
    __crt_cached_ptd_host& ptd
    )
{
    size_t retsize;
    errno_t retvalue = 0;

    if ((pwcs == nullptr && sizeInWords == 0) || (pwcs != nullptr && sizeInWords == 0))
    {
        return EINVAL;
    }

    if (pwcs != nullptr)
    {
        _RESET_STRING(pwcs, sizeInWords);
    }

    if (pConvertedChars != nullptr)
    {
        *pConvertedChars = 0;
    }

    size_t bufferSize = n > sizeInWords ? sizeInWords : n;
    if (bufferSize > INT_MAX)
    {
        return EINVAL;
    }

    retsize = _mbstowcs_l_helper(pwcs, s, bufferSize, ptd);

    if (retsize == (size_t) - 1)
    {
        if (pwcs != nullptr)
        {
            _RESET_STRING(pwcs, sizeInWords);
        }
        return EINVAL;
    }

    retsize++;

    if (pwcs != nullptr)
    {
        if (retsize > sizeInWords)
        {
            if (n != _TRUNCATE)
            {
                _RESET_STRING(pwcs, sizeInWords);
                return ERANGE;
            }
            retsize = sizeInWords;
            retvalue = STRUNCATE;
        }

        pwcs[retsize - 1] = L'\0';
    }

    if (pConvertedChars != nullptr)
    {
        *pConvertedChars = retsize;
    }

    return retvalue;
}

extern "C" errno_t __cdecl _mbstowcs_s_l(
    size_t *     pConvertedChars,
    wchar_t *    pwcs,
    size_t       sizeInWords,
    const char * s,
    size_t       n,
    _locale_t    plocinfo
    )
{
    __crt_cached_ptd_host ptd(plocinfo);
    return _mbstowcs_internal(pConvertedChars, pwcs, sizeInWords, s, n, ptd);
}

extern "C" errno_t __cdecl mbstowcs_s(
    size_t *pConvertedChars,
    wchar_t  *pwcs,
    size_t sizeInWords,
    const char *s,
    size_t n
    )
{
    __crt_cached_ptd_host ptd;
    return _mbstowcs_internal(pConvertedChars, pwcs, sizeInWords, s, n, ptd);
}
