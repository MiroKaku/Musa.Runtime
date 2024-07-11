//
// wcspbrk.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcspbrk(), which returns a pointer to the first wide character in
// 'string' that is in the 'control'.
//
#include <string.h>



extern "C" wchar_t const* __cdecl wcspbrk(
    wchar_t const* const string,
    wchar_t const* const control
    )
{
    for (wchar_t const* string_it = string; *string_it; ++string_it)
    {
        if (wcschr(control, *string_it) != nullptr)
        {
            return string_it;
        }
    }

    return nullptr;
}
