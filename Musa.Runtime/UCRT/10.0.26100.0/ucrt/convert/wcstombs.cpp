// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// wcstombs.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale with CP_UTF8 codepage.
// All locale-dependent branches are eliminated.

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>
#include <corecrt_internal_securecrt.h>
#include <errno.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>

static size_t __cdecl _wcstombs_l_helper(
    _Out_writes_(n) char *                 s,
    _In_z_          const wchar_t *        pwcs,
    _In_            size_t                 n,
    _Inout_         __crt_cached_ptd_host& ptd
    )
{
    UNREFERENCED_PARAMETER(ptd);

    // Kernel mode: always CP_UTF8
    mbstate_t state{};
    return __crt_mbstring::__wcsrtombs_utf8(s, &pwcs, n, &state, ptd);
}

extern "C" size_t __cdecl _wcstombs_l(
    char * s,
    const wchar_t * pwcs,
    size_t n,
    _locale_t plocinfo
    )
{
    __crt_cached_ptd_host ptd(plocinfo);
    return _wcstombs_l_helper(s, pwcs, n, ptd);
}

extern "C" size_t __cdecl wcstombs(
    char * s,
    const wchar_t * pwcs,
    size_t n
    )
{
    __crt_cached_ptd_host ptd;
    return _wcstombs_l_helper(s, pwcs, n, ptd);
}

static errno_t __cdecl _wcstombs_internal (
    size_t *               pConvertedChars,
    char *                 dst,
    size_t                 sizeInBytes,
    const wchar_t *        src,
    size_t                 n,
    __crt_cached_ptd_host& ptd
    )
{
    size_t retsize;
    errno_t retvalue = 0;

    if ((dst != nullptr && sizeInBytes == 0) || (dst == nullptr && sizeInBytes != 0))
    {
        return EINVAL;
    }
    if (dst != nullptr)
    {
        _RESET_STRING(dst, sizeInBytes);
    }

    if (pConvertedChars != nullptr)
    {
        *pConvertedChars = 0;
    }

    size_t bufferSize = n > sizeInBytes ? sizeInBytes : n;
    if (bufferSize > INT_MAX)
    {
        return EINVAL;
    }

    retsize = _wcstombs_l_helper(dst, src, bufferSize, ptd);

    if (retsize == (size_t)-1)
    {
        if (dst != nullptr)
        {
            _RESET_STRING(dst, sizeInBytes);
        }
        return EINVAL;
    }

    retsize++;

    if (dst != nullptr)
    {
        if (retsize > sizeInBytes)
        {
            if (n != _TRUNCATE)
            {
                _RESET_STRING(dst, sizeInBytes);
                return ERANGE;
            }
            retsize = sizeInBytes;
            retvalue = STRUNCATE;
        }

        dst[retsize - 1] = '\0';
    }

    if (pConvertedChars != nullptr)
    {
        *pConvertedChars = retsize;
    }

    return retvalue;
}

extern "C" errno_t __cdecl _wcstombs_s_l (
    size_t *pConvertedChars,
    char * dst,
    size_t sizeInBytes,
    const wchar_t * src,
    size_t n,
    _locale_t plocinfo
    )
{
    __crt_cached_ptd_host ptd(plocinfo);
    return _wcstombs_internal(pConvertedChars, dst, sizeInBytes, src, n, ptd);
}

extern "C" errno_t __cdecl wcstombs_s (
    size_t *pConvertedChars,
    char * dst,
    size_t sizeInBytes,
    const wchar_t * src,
    size_t n
    )
{
    __crt_cached_ptd_host ptd;
    return _wcstombs_internal(pConvertedChars, dst, sizeInBytes, src, n, ptd);
}
