// Kernel-mode wide string case conversion -- C locale only.
//
// wcsupr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII a-z converted.

#include <corecrt_internal.h>
#include <wchar.h>

extern "C" wchar_t * __cdecl _wcsupr_l (
    wchar_t * wsrc,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (wsrc == nullptr)
        return nullptr;

    for (wchar_t *p = wsrc; *p; ++p)
    {
        if (L'a' <= *p && *p <= L'z')
            *p -= (wchar_t)(L'a' - L'A');
    }

    return wsrc;
}

extern "C" wchar_t * __cdecl _wcsupr (
    wchar_t * wsrc
    )
{
    return _wcsupr_l(wsrc, nullptr);
}

extern "C" errno_t __cdecl _wcsupr_s_l (
    wchar_t * wsrc,
    size_t sizeInWords,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);
    UNREFERENCED_PARAMETER(sizeInWords);

    if (wsrc == nullptr)
        return EINVAL;

    for (wchar_t *p = wsrc; *p; ++p)
    {
        if (L'a' <= *p && *p <= L'z')
            *p -= (wchar_t)(L'a' - L'A');
    }

    return 0;
}

extern "C" errno_t __cdecl _wcsupr_s (
    wchar_t * wsrc,
    size_t sizeInWords
    )
{
    return _wcsupr_s_l(wsrc, sizeInWords, nullptr);
}
