// Kernel-mode MBCS secure character copy -- C locale (CP_UTF8).
//
// mbccpy_s_l.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to CP_UTF8.

#ifndef _MBCS
    #error This file should only be compiled with _MBCS defined
#endif

// Undefine macros to avoid conflicts
#undef _ismbblead_l
#undef _ismbblead
#undef _ISMBBLEAD

#include <corecrt_internal_mbstring.h>

static int __cdecl utf8_char_len(unsigned char b)
{
    if (b < 0x80)  return 1;
    if (b < 0xE0)  return 2;
    if (b < 0xF0)  return 3;
    if (b < 0xF8)  return 4;
    return 1;
}

errno_t __cdecl _mbccpy_s_l(
    unsigned char *_Dst,
    size_t _SizeInBytes,
    int *_PCopied,
    const unsigned char *_Src,
    _locale_t _Locale
    )
{
    UNREFERENCED_PARAMETER(_Locale);

    if (_PCopied != nullptr)
        *_PCopied = 0;

    if (_Dst == nullptr)
        return EINVAL;

    if (_SizeInBytes == 0)
        return EINVAL;

    if (_Src == nullptr)
    {
        *_Dst = '\0';
        return EINVAL;
    }

    int len = utf8_char_len(*_Src);

    if (_SizeInBytes < (size_t)len)
    {
        *_Dst = '\0';
        return ERANGE;
    }

    for (int i = 0; i < len; ++i)
        _Dst[i] = _Src[i];

    if (_PCopied != nullptr)
        *_PCopied = len;

    return 0;
}
