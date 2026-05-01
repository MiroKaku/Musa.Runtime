/***
*stricmp.cpp - contains case-insensitive string comp routine _stricmp
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       contains _stricmp()
*
*******************************************************************************/
#include <corecrt_internal.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>
#include "strcompare.h"

#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018 Prefast can't see that we are checking for terminal nul.

/***
*int _stricmp(lhs, rhs) - compare strings, ignore case
*
*Purpose:
*       _stricmp/_strcmpi perform a case-insensitive string comparision.
*       For differences, upper case letters are mapped to lower case.
*       Thus, "abc_" < "ABCD" since "_" < "d".
*
*Entry:
*       char *lhs, *rhs - strings to compare
*
*Return:
*       Returns <0 if lhs < rhs
*       Returns 0 if lhs = rhs
*       Returns >0 if lhs > rhs
*       Returns _NLSCMPERROR if something went wrong
*
*Exceptions:
*       Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/

extern "C" DECLSPEC_NOINLINE int __cdecl _stricmp_l (
        char const * const lhs,
        char const * const rhs,
        _locale_t    const plocinfo
        )
{
    /* validation section */
    _VALIDATE_RETURN(lhs != nullptr, EINVAL, _NLSCMPERROR);
    _VALIDATE_RETURN(rhs != nullptr, EINVAL, _NLSCMPERROR);

    unsigned char const * lhs_ptr = reinterpret_cast<unsigned char const *>(lhs);
    unsigned char const * rhs_ptr = reinterpret_cast<unsigned char const *>(rhs);

    _LocaleUpdate _loc_update(plocinfo);

    int result;
    int lhs_value;
    int rhs_value;
    do
    {
        lhs_value = _tolower_fast_internal(*lhs_ptr++, _loc_update.GetLocaleT());
        rhs_value = _tolower_fast_internal(*rhs_ptr++, _loc_update.GetLocaleT());
        result = lhs_value - rhs_value;
    }
    while (result == 0 && lhs_value != 0);

    return result;
}

extern "C" int __cdecl __ascii_stricmp (
        char const * const lhs,
        char const * const rhs
        )
{
#if defined(_M_ARM64) || defined(_M_ARM64EC) || defined(_M_HYBRID_X86_ARM64)
    return __ascii_stricmp_neon(lhs, rhs);
#else
    unsigned char const * lhs_ptr = reinterpret_cast<unsigned char const *>(lhs);
    unsigned char const * rhs_ptr = reinterpret_cast<unsigned char const *>(rhs);

    int result;
    int lhs_value;
    int rhs_value;
    do
    {
        lhs_value = *lhs_ptr++;
        rhs_value = *rhs_ptr++;
        if (lhs_value != rhs_value)
        {
            lhs_value = __ascii_tolower(lhs_value);
            rhs_value = __ascii_tolower(rhs_value);
        }
        result = lhs_value - rhs_value;
    }
    while (result == 0 && lhs_value != 0);

    return result;
#endif
}

//
// Mark _stricmp_validate_param DECLSPEC_NOINLINE to make
// _stricmp a leaf function for better performance.
//
static DECLSPEC_NOINLINE int _stricmp_validate_param (
        char const * const lhs,
        char const * const rhs
        )
{
    /* validation section */
    _VALIDATE_RETURN(lhs != nullptr, EINVAL, _NLSCMPERROR);
    _VALIDATE_RETURN(rhs != nullptr, EINVAL, _NLSCMPERROR);
    return _NLSCMPERROR;
}

extern "C" int __cdecl _stricmp (
        char const * const lhs,
        char const * const rhs
        )
{
    if (!__acrt_locale_changed())
    {
        if (lhs == nullptr || rhs == nullptr)
        {
            return _stricmp_validate_param(lhs, rhs);
        }

        return __ascii_stricmp(lhs, rhs);
    }

    return _stricmp_l(lhs, rhs, nullptr);
}
