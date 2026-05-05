// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// wcrtomb.cpp
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
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

using namespace __crt_mbstring;

/***
*errno_t _wcrtomb_internal() - Helper function to convert wide character to multibyte character.
*
*Purpose:
*       Convert a wide character into the equivalent multi-byte character.
*       In kernel mode: always uses CP_UTF8.
*
*******************************************************************************/

_Success_(return == 0)
static errno_t __cdecl _wcrtomb_internal(
                                            int*               const return_value,
    __out_bcount_z_opt(destination_count)   char*              const destination,
                                            size_t             const destination_count,
                                            wchar_t            const wchar,
                                            mbstate_t*         const state,
    _Inout_                                 __crt_cached_ptd_host&   ptd
    )
{
    _ASSERTE(destination != nullptr && destination_count > 0);

    UNREFERENCED_PARAMETER(ptd);
    UNREFERENCED_PARAMETER(destination_count);

    if (state)
    {
        state->_Wchar = 0;
    }

    // Kernel mode: always CP_UTF8
    static mbstate_t local_state{};
    int result = static_cast<int>(__crt_mbstring::__c32rtomb_utf8(destination, static_cast<char32_t>(wchar), (state != nullptr ? state : &local_state), ptd));
    if (return_value != nullptr)
    {
        *return_value = result;
    }
    if (result <= 4)
    {
        return 0;
    }
    else
    {
        return ptd.get_errno().value_or(0);
    }
}

/***
*errno_t wcrtomb_s(retValue, destination, destination_count, wchar, state) - translate wchar_t to multibyte, restartably
*
*******************************************************************************/

static errno_t __cdecl wcrtomb_s_internal(
    size_t*            const return_value,
    char*              const destination,
    size_t             const destination_count,
    wchar_t            const wchar,
    mbstate_t*         const state,
    __crt_cached_ptd_host&   ptd
    )
{
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, (destination == nullptr && destination_count == 0) || (destination != nullptr), EINVAL);

    errno_t e = 0;
    int     int_return_value = -1;
    if (destination == nullptr)
    {
        char buf[MB_LEN_MAX];
        e = _wcrtomb_internal(&int_return_value, buf, MB_LEN_MAX, wchar, state, ptd);
    }
    else
    {
        e = _wcrtomb_internal(&int_return_value, destination, destination_count, wchar, state, ptd);
    }

    if (return_value != nullptr)
    {
        *return_value = static_cast<size_t>(int_return_value);
    }

    return e;
}

extern "C" errno_t __cdecl wcrtomb_s(
    size_t*    const return_value,
    char*      const destination,
    size_t     const destination_count,
    wchar_t    const wchar,
    mbstate_t* const state
    )
{
    __crt_cached_ptd_host ptd;
    return wcrtomb_s_internal(return_value, destination, destination_count, wchar, state, ptd);
}

extern "C" size_t __cdecl wcrtomb(
    char*      const destination,
    wchar_t    const wchar,
    mbstate_t* const state
    )
{
    size_t return_value = static_cast<size_t>(-1);
    wcrtomb_s(&return_value, destination, (destination == nullptr ? 0 : MB_LEN_MAX), wchar, state);
    return return_value;
}

/* Helper shared by secure and non-secure functions. */

static size_t __cdecl _wcsrtombs_internal(
    _Pre_maybenull_ _Post_z_    char*                   destination,
    _Inout_ _Deref_prepost_z_   wchar_t const** const   source,
    _In_                        size_t                  n,
    _Out_opt_                   mbstate_t*      const   state,
    _Inout_                     __crt_cached_ptd_host&  ptd
    ) throw()
{
    _UCRT_VALIDATE_RETURN(ptd, source != nullptr, EINVAL, (size_t)-1);

    // Kernel mode: always CP_UTF8
    return __wcsrtombs_utf8(destination, source, n, state, ptd);
}

extern "C" size_t __cdecl wcsrtombs(
    char*           const destination,
    wchar_t const** const source,
    size_t          const n,
    mbstate_t*      const state
    )
{
    __crt_cached_ptd_host ptd;
    return _wcsrtombs_internal(destination, source, n, state, ptd);
}

extern "C" errno_t __cdecl wcsrtombs_s(
    size_t*         const return_value,
    char*           const destination,
    size_t          const destination_count,
    wchar_t const** const source,
    size_t          const n,
    mbstate_t*      const state
    )
{
    __crt_cached_ptd_host ptd;

    if (return_value != nullptr)
    {
        *return_value = static_cast<size_t>(-1);
    }

    _UCRT_VALIDATE_RETURN_ERRCODE(
        ptd,
        (destination == nullptr && destination_count == 0) ||
        (destination != nullptr && destination_count >  0),
    EINVAL);

    if (destination != nullptr)
    {
        _RESET_STRING(destination, destination_count);
    }

    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, source != nullptr, EINVAL);

    size_t retsize = _wcsrtombs_internal(destination, source, (n > destination_count ? destination_count : n), state, ptd);
    if (retsize == static_cast<size_t>(-1))
    {
        if (destination != nullptr)
        {
            _RESET_STRING(destination, destination_count);
        }

        return ptd.get_errno().value_or(0);
    }

    ++retsize; // Account for the null terminator

    if (destination != nullptr)
    {
        if (retsize > destination_count)
        {
            _RESET_STRING(destination, destination_count);
            _UCRT_VALIDATE_RETURN_ERRCODE(ptd, retsize <= destination_count, ERANGE);
        }

        destination[retsize - 1] = '\0';
    }

    if (return_value != nullptr)
    {
        *return_value = retsize;
    }

    return 0;
}

// Converts a wide character into a one-byte character
extern "C" int __cdecl wctob(wint_t const wchar)
{
    if (wchar == WEOF)
    {
        return EOF;
    }

    int  return_value = -1;
    char local_buffer[MB_LEN_MAX];

    mbstate_t state{};
    __crt_cached_ptd_host ptd;
    errno_t const e = _wcrtomb_internal(&return_value, local_buffer, MB_LEN_MAX, wchar, &state, ptd);
    if (e == 0 && return_value == 1)
    {
        return local_buffer[0];
    }

    return EOF;
}

// UTF-8 wide string to multibyte string conversion
size_t __cdecl __crt_mbstring::__wcsrtombs_utf8(char* dst, const wchar_t** src, size_t len, mbstate_t* ps, __crt_cached_ptd_host& ptd)
{
    UNREFERENCED_PARAMETER(ptd);

    const wchar_t* current_src = *src;
    char buf[MB_LEN_MAX];

    if (dst != nullptr)
    {
        char* current_dest = dst;
        const wchar_t* start_of_code_point = current_src;
        for (;;)
        {
            char* temp;
            if (len < 4)
            {
                temp = buf;
            }
            else
            {
                temp = current_dest;
            }
            const size_t retval = __crt_mbstring::__c16rtomb_utf8(temp, *current_src, ps, ptd);

            if (retval == __crt_mbstring::INVALID)
            {
                *src = start_of_code_point;
                return retval;
            }

            if (temp == current_dest)
            {
                // wrote in-place
            }
            else if (len < retval)
            {
                current_src = start_of_code_point;
                break;
            }
            else
            {
                memcpy(current_dest, temp, retval);
            }

            if (retval > 0 && current_dest[retval - 1] == '\0')
            {
                current_src = nullptr;
                current_dest += retval - 1;
                break;
            }

            ++current_src;
            if (retval > 0)
            {
                start_of_code_point = current_src;
            }

            len -= retval;
            current_dest += retval;
        }
        *src = current_src;
        return current_dest - dst;
    }
    else
    {
        size_t total_count = 0;
        for (;;)
        {
            const size_t retval = __crt_mbstring::__c16rtomb_utf8(buf, *current_src, ps, ptd);
            if (retval == __crt_mbstring::INVALID)
            {
                return retval;
            }
            else if (retval > 0 && buf[retval - 1] == '\0')
            {
                total_count += retval - 1;
                break;
            }
            total_count += retval;
            ++current_src;
        }
        return total_count;
    }
}
