//
// output.cpp -- Kernel-mode stdio output wrappers
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Symbol sourcing:
//   base SDK output.cpp:  __stdio_common_vsprintf (printf engine), __stdio_common_vswprintf
//   base SDK input.cpp:   __stdio_common_vsscanf (scanf engine)
//   this overlay:         _vscprintf, _vscwprintf (→ _vsnprintf / _vsnwprintf)
//                         _vsnprintf          (→ __stdio_common_vsprintf)
//                         snprintf            (→ _vsnprintf)
//                         sscanf              (→ __stdio_common_vsscanf)
//   Self-contained -- no ntoskrnl dependency for printf/scanf.
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

// ---- _vsnprintf -- delegates to base SDK __stdio_common_vsprintf ----

extern "C" int __cdecl _vsnprintf(char* buf, size_t n, const char* fmt, va_list args)
{
    return __stdio_common_vsprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, buf, n, fmt, nullptr, args);
}

// ---- _vsnwprintf -- delegates to base SDK __stdio_common_vswprintf ----

extern "C" int __cdecl _vsnwprintf(wchar_t* buf, size_t n, const wchar_t* fmt, va_list args)
{
    return __stdio_common_vswprintf(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS, buf, n, fmt, nullptr, args);
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

extern "C" int __cdecl snprintf(char* buf, size_t n, const char* fmt, ...)
{
    va_list args;
    __crt_va_start(args, fmt);
    int const r = _vsnprintf(buf, n, fmt, args);
    __crt_va_end(args);
    return r;
}

_CRT_END_C_HEADER

#pragma warning(pop)
