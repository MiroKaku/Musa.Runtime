// Kernel-mode wide string case conversion -- C locale only.
//
// wcslwr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII A-Z converted.

#include <corecrt_internal.h>
#include <wchar.h>

extern "C" wchar_t * __cdecl _wcslwr_l (
    wchar_t * wsrc,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (wsrc == nullptr)
        return nullptr;

    for (wchar_t *p = wsrc; *p; ++p)
    {
        if (L'A' <= *p && *p <= L'Z')
            *p += (wchar_t)L'a' - (wchar_t)L'A';
    }

    return wsrc;
}

extern "C" wchar_t * __cdecl _wcslwr (
    wchar_t * wsrc
    )
{
    return _wcslwr_l(wsrc, nullptr);
}

extern "C" errno_t __cdecl _wcslwr_s_l (
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
        if (L'A' <= *p && *p <= L'Z')
            *p += (wchar_t)L'a' - (wchar_t)L'A';
    }

    return 0;
}

extern "C" errno_t __cdecl _wcslwr_s (
    wchar_t * wsrc,
    size_t sizeInWords
    )
{
    return _wcslwr_s_l(wsrc, sizeInWords, nullptr);
}
