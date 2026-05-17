// Kernel-mode wide string collation -- C locale only.
//
// wcsncoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. wcsncoll() resolves to wcsncmp().

#include <corecrt_internal.h>
#include <wchar.h>

extern "C" int __cdecl _wcsncoll_l (
    const wchar_t *_string1,
    const wchar_t *_string2,
    size_t count,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (_string1 == nullptr || _string2 == nullptr || count > INT_MAX)
    {
        return _NLSCMPERROR;
    }

    if (!count)
        return 0;

    // Kernel mode: C locale, wcsncoll = wcsncmp
    return wcsncmp(_string1, _string2, count);
}

extern "C" int __cdecl _wcsncoll (
    const wchar_t *_string1,
    const wchar_t *_string2,
    size_t count
    )
{
    return _wcsncoll_l(_string1, _string2, count, nullptr);
}
