// Kernel-mode string case conversion -- C locale only.
//
// strlwr.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Only ASCII A-Z converted.

#include <corecrt_internal.h>
#include <string.h>

extern "C" char * __cdecl _strlwr_l (
    char * string,
    _locale_t plocinfo
    )
{
    UNREFERENCED_PARAMETER(plocinfo);

    if (string == nullptr)
        return nullptr;

    for (char *cp = string; *cp; ++cp)
    {
        if ('A' <= *cp && *cp <= 'Z')
            *cp += 'a' - 'A';
    }

    return string;
}

extern "C" char * __cdecl _strlwr (
    char * string
    )
{
    return _strlwr_l(string, nullptr);
}

extern "C" errno_t __cdecl _strlwr_s_l (
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
        if ('A' <= *cp && *cp <= 'Z')
            *cp += 'a' - 'A';
    }

    return 0;
}

extern "C" errno_t __cdecl _strlwr_s(
    char * string,
    size_t sizeInBytes
    )
{
    return _strlwr_s_l(string, sizeInBytes, nullptr);
}
