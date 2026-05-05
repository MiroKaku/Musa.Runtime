// Kernel-mode string transformation -- C locale only.
//
// strxfrm.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strxfrm() = copy + return length.

#include <corecrt_internal.h>
#include <string.h>

extern "C" size_t __cdecl _strxfrm_l (
    char *_string1,
    const char *_string2,
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

    if (_string1 != nullptr && _count > 0)
    {
        *_string1 = '\0';
    }

    // Kernel mode: C locale, strxfrm = simple copy
    size_t len = strlen(_string2);

    if (_string1 != nullptr)
    {
        size_t copy_len = len < _count ? len : _count - 1;
        memcpy(_string1, _string2, copy_len);
        _string1[copy_len] = '\0';

        if (len >= _count)
        {
            *_string1 = '\0';
        }
    }

    return len;
}

extern "C" size_t __cdecl strxfrm (
    char *_string1,
    const char *_string2,
    size_t _count
    )
{
    return _strxfrm_l(_string1, _string2, _count, nullptr);
}
