// Kernel-mode MBCS character classification -- C locale only.
//
// ismblgl.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale. Checks valid lead/trail byte pair.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

#include <corecrt_internal_mbstring.h>

extern "C" int __cdecl _ismbclegal_l(unsigned int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    // Check if high byte is a lead byte and low byte is a trail byte using static table
    return (_ismbblead(c >> 8) && _ismbbtrail(c & 0x0FF));
}

extern "C" int __cdecl _ismbclegal(unsigned int const c)
{
    return _ismbclegal_l(c, nullptr);
}
