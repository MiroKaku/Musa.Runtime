// Kernel-mode localeconv -- C locale only.
//
// localeconv.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Always returns &__acrt_lconv_c.

#include <corecrt_internal.h>
#include <locale.h>

extern "C" struct lconv* __cdecl localeconv()
{
    return &__acrt_lconv_c;
}
