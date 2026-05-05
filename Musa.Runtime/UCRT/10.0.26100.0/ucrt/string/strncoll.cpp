// Kernel-mode string collation -- C locale only.
//
// strncoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strncoll() resolves to strncmp().

#include <corecrt_internal.h>
#include <string.h>

extern "C" int __cdecl _strncoll_l (
    const char *_string1,
    const char *_string2,
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

    // Kernel mode: C locale, strncoll = strncmp
    return strncmp(_string1, _string2, count);
}

extern "C" int __cdecl _strncoll (
    const char *_string1,
    const char *_string2,
    size_t count
    )
{
    return _strncoll_l(_string1, _string2, count, nullptr);
}
