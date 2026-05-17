// Kernel-mode wide string transformation -- C locale only.
//
// wcsxfrm.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. wcsxfrm() = copy + return length.

#include <corecrt_internal.h>
#include <wchar.h>

extern "C" size_t __cdecl _wcsxfrm_l (
    wchar_t *_string1,
    const wchar_t *_string2,
    size_t _count,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (_count > INT_MAX || _string2 == nullptr)
    {
        return INT_MAX;
    }

    if (_string1 == nullptr && _count != 0)
    {
        return INT_MAX;
    }

    // Kernel mode: C locale, wcsxfrm = simple copy
    size_t len = wcslen(_string2);

    if (_string1 != nullptr)
    {
        size_t copy_len = len < _count ? len : _count - 1;
        wmemcpy(_string1, _string2, copy_len);
        _string1[copy_len] = L'\0';

        if (len >= _count)
        {
            *_string1 = L'\0';
        }
    }

    return len;
}

extern "C" size_t __cdecl wcsxfrm (
    wchar_t *_string1,
    const wchar_t *_string2,
    size_t _count
    )
{
    return _wcsxfrm_l(_string1, _string2, _count, nullptr);
}
