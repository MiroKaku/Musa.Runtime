//
// output.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The standard _output functions, which perform formatted output to a stream.
//

#pragma warning(push)
#pragma warning(disable: 4996)

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

_CRT_BEGIN_C_HEADER

_Check_return_
_CRT_STDIO_INLINE int __CRTDECL _vscprintf(
    _In_z_ _Printf_format_string_ char const* const _Format,
    va_list           _ArgList
)
{
    return _vsnprintf(0, 0, _Format, _ArgList);
}

_Success_(return >= 0)
_Check_return_
_CRT_STDIO_INLINE int __CRTDECL _vscwprintf(
    _In_z_ _Printf_format_string_ wchar_t const* const _Format,
    va_list              _ArgList
)
{
    return _vsnwprintf(0, 0, _Format, _ArgList);
}

_CRT_END_C_HEADER

#pragma warning(pop)

// ---- snprintf wrapper (needed when _NO_CRT_STDIO_INLINE is defined) ----

extern "C" int __cdecl _vsnprintf(char* buf, size_t n, const char* fmt, va_list args)
{
    return __stdio_common_vsprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, buf, n, fmt, nullptr, args);
}

extern "C" int __cdecl snprintf(char* buf, size_t n, const char* fmt, ...)
{
    va_list args;
    __crt_va_start(args, fmt);
    int const r = _vsnprintf(buf, n, fmt, args);
    __crt_va_end(args);
    return r;
}

// ---- sscanf wrapper (needed when _NO_CRT_STDIO_INLINE is defined) ----

extern "C" int __cdecl sscanf(const char* buf, const char* fmt, ...)
{
    va_list args;
    __crt_va_start(args, fmt);
    int const r = __stdio_common_vsscanf(_CRT_INTERNAL_LOCAL_SCANF_OPTIONS, buf, static_cast<size_t>(-1), fmt, nullptr, args);
    __crt_va_end(args);
    return r;
}
extern "C" int __cdecl __stdio_common_vsprintf(
    unsigned __int64 options, char* buffer, size_t buffer_count,
    const char* format, _locale_t locale, va_list arglist)
{
    UNREFERENCED_PARAMETER(options);
    UNREFERENCED_PARAMETER(locale);
    if (!buffer || buffer_count == 0) return -1;
    int const r = _vsnprintf(buffer, buffer_count, format, arglist);
    return r < 0 ? -1 : r;
}
