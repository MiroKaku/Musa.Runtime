// Kernel-mode wide character case conversion -- C locale only.
//
// towlower.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII A-Z converted.

#include <corecrt_internal.h>
#include <ctype.h>

extern "C" wint_t __cdecl _towlower_l(
    wint_t const c,
    _locale_t const plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (c == WEOF)
        return c;

    // Kernel mode: C locale, only ASCII
    if (c < 128)
    {
        return _towlower_fast_internal(static_cast<unsigned char>(c), nullptr);
    }

    return c;
}

extern "C" wint_t __cdecl towlower (
    wint_t c
    )
{
    return _towlower_l(c, nullptr);
}
