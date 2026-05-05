// Kernel-mode MBCS string copy (safe) -- C locale only.
//
// mbsncpy_s.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Redirect to _mbsncpy_s_l.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

_REDIRECT_TO_L_VERSION_4(errno_t, _mbsncpy_s, unsigned char *, size_t, const unsigned char *, size_t)
