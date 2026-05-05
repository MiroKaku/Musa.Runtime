// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// mblen.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The mblen() and _mblen_l() functions, which return the number of bytes
// contained in a multibyte character.
// In kernel mode, locale is fixed to C locale with CP_UTF8 codepage.
// All locale-dependent branches are eliminated.
//
#define _ALLOW_OLD_VALIDATE_MACROS
#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>

using namespace __crt_mbstring;

// Computes the number of bytes contained in a multibyte character.  If the string
// is null, zero is returned to indicate that we support only state-independent
// character encodings.  If the next max_count bytes of the string are not a valid
// multibyte character, -1 is returned.  Otherwise, the number of bytes that
// compose the next multibyte character are returned.
//
// Kernel mode: always uses UTF-8 encoding.
static int __cdecl _mblen_internal(
    char const*        const string,
    size_t             const max_count
    )
{
    mbstate_t internal_state{};
    if (!string || *string == '\0' || max_count == 0)
    {
        internal_state = {};
        return 0;
    }

    __crt_cached_ptd_host ptd;

    int result = static_cast<int>(__mbrtowc_utf8(nullptr, string, max_count, &internal_state, ptd));
    if (result < 0)
    {
        result = -1;
    }
    return result;
}

extern "C" int __cdecl _mblen_l(
    char const* const string,
    size_t      const max_count,
    _locale_t   const locale
    )
{
    UNREFERENCED_PARAMETER(locale);
    return _mblen_internal(string, max_count);
}



extern "C" int __cdecl mblen(
    char const* const string,
    size_t      const max_count
    )
{
    return _mblen_internal(string, max_count);
}
