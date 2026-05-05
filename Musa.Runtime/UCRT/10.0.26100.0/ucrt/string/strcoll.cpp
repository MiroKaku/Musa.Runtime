// Kernel-mode string collation -- C locale only.
//
// strcoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. strcoll() resolves to strcmp().

#include <corecrt_internal.h>
#include <string.h>

extern "C" int __cdecl _strcoll_l (
    const char *_string1,
    const char *_string2,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (_string1 == nullptr || _string2 == nullptr)
    {
        return _NLSCMPERROR;
    }

    // Kernel mode: C locale, strcoll = strcmp
    return strcmp(_string1, _string2);
}

extern "C" int __cdecl strcoll (
    const char *_string1,
    const char *_string2
    )
{
    return _strcoll_l(_string1, _string2, nullptr);
}
