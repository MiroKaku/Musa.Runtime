// Kernel-mode string comparison -- C locale only.
//
// strnicmp.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. _strnicmp_l uses ASCII tolower.

#include <corecrt_internal.h>
#include <ctype.h>
#include <string.h>

static __forceinline int tolower_ascii(int c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

extern "C" DECLSPEC_NOINLINE int __cdecl _strnicmp_l (
    char const * const lhs,
    char const * const rhs,
    size_t       const count,
    _locale_t    const plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (lhs == nullptr || rhs == nullptr || count > INT_MAX)
    {
        return _NLSCMPERROR;
    }

    if (count == 0)
    {
        return 0;
    }

    unsigned char const * lhs_ptr = reinterpret_cast<unsigned char const *>(lhs);
    unsigned char const * rhs_ptr = reinterpret_cast<unsigned char const *>(rhs);

    int result;
    int lhs_value;
    int rhs_value;
    size_t remaining = count;
    do
    {
        lhs_value = tolower_ascii(*lhs_ptr++);
        rhs_value = tolower_ascii(*rhs_ptr++);
        result = lhs_value - rhs_value;
    }
    while (result == 0 && lhs_value != 0 && --remaining != 0);

    return result;
}

extern "C" int __cdecl __ascii_strnicmp (
    char const * const lhs,
    char const * const rhs,
    size_t       const count
    )
{
    return _strnicmp_l(lhs, rhs, count, nullptr);
}

extern "C" int __cdecl _strnicmp (
    char const * const lhs,
    char const * const rhs,
    size_t       const count
    )
{
    return _strnicmp_l(lhs, rhs, count, nullptr);
}
