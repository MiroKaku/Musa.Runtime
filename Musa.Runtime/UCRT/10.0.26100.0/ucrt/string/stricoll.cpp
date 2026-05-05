// Kernel-mode string collation -- C locale only.
//
// stricoll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. stricoll() resolves to stricmp().

#include <corecrt_internal.h>
#include <string.h>

extern "C" int __cdecl _stricoll_l (
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

    // Kernel mode: C locale, stricoll = stricmp
    return _stricmp(_string1, _string2);
}

extern "C" int __cdecl _stricoll (
    const char *_string1,
    const char *_string2
    )
{
    return _stricoll_l(_string1, _string2, nullptr);
}
