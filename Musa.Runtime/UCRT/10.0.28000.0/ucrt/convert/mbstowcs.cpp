// Kernel-mode mbstowcs -- delegates to MultiByteToWideChar (CP_UTF8).
//
// mbstowcs.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to UTF-8. mbstowcs/_mbstowcs_l/mbstowcs_s
// just route to MultiByteToWideChar provided by Musa.Core.

#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>
#include <corecrt_internal_securecrt.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>

/* Helper shared by secure and non-secure functions */
//
// Returns: count of wide chars produced (NOT including null terminator)
//          (size_t)-1 on encoding error.
// When pwcs == nullptr: returns required count (excluding null), n is ignored.
// When pwcs != nullptr: writes up to n chars; if conversion stops at source
//          null and n > count, also writes the null terminator.
static size_t __cdecl _mbstowcs_l_helper(
    _Out_writes_opt_z_(n)               wchar_t *              pwcs,
    _In_reads_or_z_(n) _Pre_z_          const char *           s,
    _In_                                size_t                 n,
    _In_opt_                            __crt_cached_ptd_host& ptd
    ) throw()
{
    UNREFERENCED_PARAMETER(ptd);

    if (s == nullptr)
    {
        return (size_t)-1;
    }

    if (pwcs && n == 0)
    {
        return 0;
    }

    // Sizing pass: caller wants the required count, no buffer.
    if (pwcs == nullptr)
    {
        // -1 for length means input is null-terminated and we want full size
        // including the null terminator. Subtract 1 for the standard return
        // (excluding the terminator).
        int const required = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
        if (required <= 0)
        {
            return (size_t)-1;
        }
        return static_cast<size_t>(required - 1);
    }

    // Convert into the user's buffer.
    if (n > INT_MAX)
    {
        n = INT_MAX;
    }

    int const written = MultiByteToWideChar(
        CP_UTF8, 0, s, -1, pwcs, static_cast<int>(n));
    if (written > 0)
    {
        // Stopped at source null; null terminator already written; return
        // count excluding the terminator (per C standard).
        return static_cast<size_t>(written - 1);
    }

    // Output buffer too small. Convert exactly n bytes worth of input without
    // a terminator -- find how many chars from s fit, and convert them.
    // Using -1 is wrong here because it tries to include the terminator. So
    // fall back to converting one char at a time. This branch is rare: normal
    // mbstowcs callers either size first or pass an adequately large buffer.
    int const required = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (required <= 0)
    {
        return (size_t)-1;
    }

    // Convert chunks: fill exactly n wide chars without a null terminator.
    size_t produced = 0;
    const char* p = s;
    while (produced < n && *p)
    {
        // Try converting starting from *p, computing how many bytes it takes
        // for one or more wide chars while keeping output <= n - produced.
        // Simplest reliable approach: scan one byte at a time using lead-byte
        // length; UTF-8 lead-byte length:
        unsigned char lead = static_cast<unsigned char>(*p);
        int byte_len;
        if (lead < 0x80)        byte_len = 1;
        else if ((lead & 0xE0) == 0xC0) byte_len = 2;
        else if ((lead & 0xF0) == 0xE0) byte_len = 3;
        else if ((lead & 0xF8) == 0xF0) byte_len = 4;
        else return (size_t)-1; // invalid lead byte

        int const wchars_needed = (byte_len == 4) ? 2 : 1;
        if (produced + static_cast<size_t>(wchars_needed) > n)
        {
            break;
        }

        int const w = MultiByteToWideChar(
            CP_UTF8, 0, p, byte_len, pwcs + produced,
            static_cast<int>(n - produced));
        if (w <= 0)
        {
            return (size_t)-1;
        }
        produced += static_cast<size_t>(w);
        p += byte_len;
    }
    return produced;
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
