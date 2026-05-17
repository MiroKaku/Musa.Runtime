// Kernel-mode wide string collation -- C locale only.
//
// wcsicoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. wcsicoll() resolves to wcsicmp().

#include <corecrt_internal.h>
#include <wchar.h>


extern "C" int __cdecl _wcsicoll_l (
    const wchar_t *_string1,
    const wchar_t *_string2,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (_string1 == nullptr || _string2 == nullptr)
    {
        return _NLSCMPERROR;
    }

    // Kernel mode: C locale, wcsicoll = wcsicmp
    return __ascii_wcsicmp(_string1, _string2);
}

extern "C" int __cdecl _wcsicoll (
    const wchar_t *_string1,
    const wchar_t *_string2
    )
{
    return _wcsicoll_l(_string1, _string2, nullptr);
}
