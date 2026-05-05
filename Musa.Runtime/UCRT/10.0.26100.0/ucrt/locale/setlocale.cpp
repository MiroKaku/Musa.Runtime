// Kernel-mode setlocale stub -- C locale only.
//
// setlocale.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. setlocale() returns nullptr for
// any attempt to change locale, or "C" for query.

#include <corecrt_internal.h>
#include <locale.h>
#include <stdlib.h>

extern "C" char* __cdecl setlocale(int _category, char const* _locale)
{
    UNREFERENCED_PARAMETER(_category);

    // Query only: return "C"
    if (_locale == nullptr)
    {
        return const_cast<char*>("C");
    }

    // Cannot change locale in kernel mode
    errno = ENOTSUP;
    return nullptr;
}
