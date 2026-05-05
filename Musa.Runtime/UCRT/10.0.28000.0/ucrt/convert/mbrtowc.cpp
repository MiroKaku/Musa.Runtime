// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// mbrtowc.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale with CP_UTF8 codepage.
// All locale-dependent branches are eliminated.

#define _ALLOW_OLD_VALIDATE_MACROS
#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>
#include <corecrt_internal_securecrt.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <uchar.h>
#include <wchar.h>

using namespace __crt_mbstring;

/***
*errno_t _mbrtowc_internal() - Helper function to convert multibyte char to wide character.
*
*Purpose:
*       Convert a multi-byte character into the equivalent wide character.
*       In kernel mode: always uses CP_UTF8.
*
*******************************************************************************/

_Success_(return != 0)
_Post_satisfies_(*pRetValue <= _String_length_(s))
static errno_t __cdecl _mbrtowc_internal(
    _Inout_ _Out_range_(<=, 1)              int *                  pRetValue,
    _Pre_maybenull_ _Out_writes_opt_z_(1)   wchar_t *              dst,
    _In_opt_z_                              const char *           s,
    _In_                                    size_t                 n,
    _Inout_                                 mbstate_t *            pmbst,
    _Inout_                                 __crt_cached_ptd_host& ptd
    ) throw()
{
    _ASSERTE(pmbst != nullptr);
    _ASSIGN_IF_NOT_NULL(dst, 0);

    if (!s || n == 0)
    {
        /* indicate do not have state-dependent encodings,
        handle zero length string */
        _ASSIGN_IF_NOT_NULL(pRetValue, 0);
        return 0;
    }

    if (!*s)
    {
        /* handle nullptr char */
        _ASSIGN_IF_NOT_NULL(pRetValue, 0);
        return 0;
    }

    UNREFERENCED_PARAMETER(ptd);

    // Kernel mode: always CP_UTF8
    const size_t retval = __mbrtowc_utf8(dst, s, n, pmbst, ptd);
    _ASSIGN_IF_NOT_NULL(pRetValue, static_cast<int>(retval));
    return ptd.get_errno().value_or(0);
}


/***
*wint_t btowc(c) - translate single byte to wide char
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

extern "C" wint_t __cdecl btowc(
    int c
    )
{
    if (c == EOF)
    {
        return WEOF;
    }
    else
    {
        /* convert as one-byte string */
        char ch = (char) c;
        mbstate_t mbst = {};
        wchar_t wc = 0;
        int retValue = -1;

        __crt_cached_ptd_host ptd;
        _mbrtowc_internal(&retValue, &wc, &ch, 1, &mbst, ptd);
        return (retValue < 0 ? WEOF : wc);
    }
}


/***
*size_t mbrlen(s, n, pst) - determine next multibyte code, restartably
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

extern "C" size_t __cdecl mbrlen(
    const char *s,
    size_t n,
    mbstate_t *pst
    )
{
    static mbstate_t mbst = {};
    int retValue = -1;

    __crt_cached_ptd_host ptd;
    _mbrtowc_internal(&retValue, nullptr, s, n, (pst != nullptr ? pst : &mbst), ptd);
    return retValue;
}


/***
*size_t mbrtowc(pwc, s, n, pst) - translate multibyte to wchar_t, restartably
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

extern "C" size_t __cdecl mbrtowc(
    wchar_t *dst,
    const char *s,
    size_t n,
    mbstate_t *pst
    )
{
    static mbstate_t mbst = {};
    int retValue = -1;

    __crt_cached_ptd_host ptd;

    if (s != nullptr)
    {
        _mbrtowc_internal(&retValue, dst, s, n, (pst != nullptr ? pst : &mbst), ptd);
    }
    else
    {
        _mbrtowc_internal(&retValue, nullptr, "", 1, (pst != nullptr ? pst : &mbst), ptd);
    }
    return retValue;
}


/***
*size_t mbsrtowcs(wcs, ps, n, pst) - translate multibyte string to wide,
*       restartably
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

/* Helper function shared by the secure and non-secure versions. */

_Success_(return == 0)
static size_t __cdecl _mbsrtowcs_helper(
    _Out_writes_opt_z_(n)               wchar_t *              wcs,
    _Deref_pre_opt_z_                   const char **          ps,
    _In_                                size_t                 n,
    _Inout_                             mbstate_t *            pst,
    _Inout_                             __crt_cached_ptd_host& ptd
    ) throw()
{
    /* validation section */
    _UCRT_VALIDATE_RETURN(ptd, ps != nullptr, EINVAL, (size_t) - 1);

    static mbstate_t mbst = {};
    const char *s = *ps;
    int i = 0;
    size_t nwc = 0;

    // Use the static cached state if necessary
    if (pst == nullptr)
    {
        pst = &mbst;
    }

    // Kernel mode: always CP_UTF8
    return __mbsrtowcs_utf8(wcs, ps, n, pst, ptd);
}

/***
*size_t mbsrtowcs() - Convert multibyte char string to wide char string.
*
*Purpose:
*       Convert a multi-byte char string into the equivalent wide char string.
*       Same as mbsrtowcs_s(), but the destination may not be null terminated.
*       If there's not enough space, we return EINVAL.
*
*Entry:
*       wchar_t *pwcs = pointer to destination wide character string buffer
*       const char **s = pointer to source multibyte character string
*       size_t n = maximum number of wide characters to store (not including the terminating null character)
*       mbstate_t *pst = pointer to the conversion state
*
*Exit:
*       The nunber if wide characters written to *wcs, not including any terminating null character)
*
*Exceptions:
*       Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/
extern "C" size_t __cdecl mbsrtowcs(
    wchar_t *     wcs,
    const char ** ps,
    size_t        n,
    mbstate_t *   pst
    )
{
    /* Call a non-deprecated helper to do the work. */
    __crt_cached_ptd_host ptd;
    return _mbsrtowcs_helper(wcs, ps, n, pst, ptd);
}


/***
*errno_t mbsrtowcs_s() - Convert multibyte char string to wide char string.
*
*Purpose:
*       Convert a multi-byte char string into the equivalent wide char string.
*       Same as mbsrtowcs(), but the destination is ensured to be null terminated.
*       If there's not enough space, we return EINVAL.
*
*Entry:
*       size_t *pRetValue = Number of bytes modified including the terminating nullptr
*                           This pointer can be nullptr.
*       wchar_t *pwcs = pointer to destination wide character string buffer
*       size_t sizeInWords = size of the destination buffer
*       const char **s = pointer to source multibyte character string
*       size_t n = maximum number of wide characters to store (not including the terminating null character)
*       mbstate_t *pst = pointer to the conversion state
*
*Exit:
*       The error code.
*
*Exceptions:
*       Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/

static errno_t __cdecl mbsrtowcs_s_internal(
    size_t *               pRetValue,
    wchar_t *              dst,
    size_t                 sizeInWords,
    const char **          ps,
    size_t                 n,
    mbstate_t *            pmbst,
    __crt_cached_ptd_host& ptd
    )
{
    size_t retsize;

    /* validation section */
    _ASSIGN_IF_NOT_NULL(pRetValue, (size_t) - 1);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, (dst == nullptr && sizeInWords == 0) || (dst != nullptr && sizeInWords > 0), EINVAL);
    if (dst != nullptr)
    {
        _RESET_STRING(dst, sizeInWords);
    }
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, ps != nullptr, EINVAL);

    /* Call a non-deprecated helper to do the work. */

    retsize = _mbsrtowcs_helper(dst, ps, (n > sizeInWords ? sizeInWords : n), pmbst, ptd);

    if (retsize == (size_t) - 1)
    {
        if (dst != nullptr)
        {
            _RESET_STRING(dst, sizeInWords);
        }
        return ptd.get_errno().value_or(0);
    }

    /* count the null terminator */
    retsize++;

    if (dst != nullptr)
    {
        /* return error if the string does not fit */
        if (retsize > sizeInWords)
        {
            _RESET_STRING(dst, sizeInWords);
            _UCRT_VALIDATE_RETURN_ERRCODE(ptd, sizeInWords <= retsize, ERANGE);
        }
        else
        {
            /* ensure the string is null terminated */
            dst[retsize - 1] = '\0';
        }
    }

    _ASSIGN_IF_NOT_NULL(pRetValue, retsize);

    return 0;
}

extern "C" errno_t __cdecl mbsrtowcs_s(
    size_t *      pRetValue,
    wchar_t *     dst,
    size_t        sizeInWords,
    const char ** ps,
    size_t        n,
    mbstate_t *   pmbst
    )
{
    __crt_cached_ptd_host ptd;
    return mbsrtowcs_s_internal(pRetValue, dst, sizeInWords, ps, n, pmbst, ptd);
}
