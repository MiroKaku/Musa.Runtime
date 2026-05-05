// Kernel-mode string case conversion -- C locale only.
//
// strupr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII a-z converted.

#include <corecrt_internal.h>
#include <string.h>

extern "C" char * __cdecl _strupr_l (
    char * string,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (string == nullptr)
        return nullptr;

    for (char *cp = string; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp -= 'a' - 'A';
    }

    return string;
}

extern "C" char * __cdecl _strupr (
    char * string
    )
{
    return _strupr_l(string, nullptr);
}

extern "C" errno_t __cdecl _strupr_s_l (
    char * string,
    size_t sizeInBytes,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);
    UNREFERENCED_PARAMETER(sizeInBytes);

    if (string == nullptr)
        return EINVAL;

    for (char *cp = string; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp -= 'a' - 'A';
    }

    return 0;
}

extern "C" errno_t __cdecl _strupr_s (
    char * string,
    size_t sizeInBytes
    )
{
    return _strupr_s_l(string, sizeInBytes, nullptr);
}
