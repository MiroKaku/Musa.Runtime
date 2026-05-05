// Kernel-mode wide character case conversion -- C locale only.
//
// towupper.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII a-z converted.

#include <corecrt_internal.h>
#include <ctype.h>

extern "C" wint_t __cdecl _towupper_l(
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
        return _towupper_fast_internal(static_cast<unsigned char>(c), nullptr);
    }

    return c;
}

extern "C" wint_t __cdecl towupper (
    wint_t c
    )
{
    return _towupper_l(c, nullptr);
}
