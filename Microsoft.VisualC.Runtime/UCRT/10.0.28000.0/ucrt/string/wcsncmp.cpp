//
// wcsncmp.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcsncmp(), which compares two wide character strings, determining
// their ordinal order.  The function tests at most 'count' characters.
//
// Note that the comparison is performed with unsigned elements (wchar_t is
// unsigned in this implementation), so the null character (0) is less than
// all other characters.
//
// Returns:
//  * a negative value if a  < b
//  * zero             if a == b
//  * a positive value if a  > b
//
#include <corecrt_internal.h>
#include <ctype.h>
#include <string.h>
#define CASE_SENSITIVE_CMP
#define BOUNDED_CMP
#include "strcompare.h"


#ifdef _M_ARM
    #pragma function(wcsncmp)
#endif

extern "C" int __cdecl wcsncmp(
    wchar_t const* a,
    wchar_t const* b,
    size_t         count
    )
{
#if defined(_M_ARM64) || defined(_M_ARM64EC) || defined(_M_HYBRID_X86_ARM64)
    return __ascii_wcsicmp_neon(a, b, count);
#else
    if (count == 0)
        return 0;

    while (--count != 0 && *a && *a == *b)
    {
        ++a;
        ++b;
    }

    return static_cast<int>(*a - *b);
#endif
}
