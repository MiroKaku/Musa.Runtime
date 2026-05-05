// Kernel-mode locale conversion functions -- UTF-8 / C locale only.
//
// strtod.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In kernel mode, locale is fixed to C locale with CP_UTF8 codepage.
// All locale-dependent branches are eliminated.

#define _ALLOW_OLD_VALIDATE_MACROS
#include <corecrt_internal_strtox.h>
#include <stdlib.h>

template <typename FloatingType, typename Character>
static FloatingType __cdecl common_strtod_l(
    _In_z_                      Character const*    const   string,
    _Out_opt_ _Deref_post_z_    Character**         const   end_ptr,
                                _locale_t           const   locale
    ) throw()
{
    if (end_ptr)
    {
        *end_ptr = const_cast<Character*>(string);
    }

    if (string == nullptr)
    {
        return 0.0;
    }

    // Kernel mode: always use initial locale (C locale, CP_UTF8)
    __crt_cached_ptd_host ptd(locale);

    FloatingType result{};
    SLD_STATUS const status = __crt_strtox::parse_floating_point(
        ptd,
        __crt_strtox::make_c_string_character_source(string, end_ptr),
        &result);

    if (status == SLD_OVERFLOW || status == SLD_UNDERFLOW)
    {
        errno = ERANGE;
    }

    return result;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Narrow Strings => Floating Point
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" float __cdecl strtof(
    char const* const string,
    char**      const end_ptr
    )
{
    return common_strtod_l<float>(string, end_ptr, nullptr);
}

extern "C" float __cdecl _strtof_l(
    char const* const string,
    char**      const end_ptr,
    _locale_t   const locale
    )
{
    return common_strtod_l<float>(string, end_ptr, locale);
}

extern "C" double __cdecl strtod(
    char const* const string,
    char**      const end_ptr
    )
{
    return common_strtod_l<double>(string, end_ptr, nullptr);
}

extern "C" double __cdecl _strtod_l(
    char const* const string,
    char**      const end_ptr,
    _locale_t   const locale
    )
{
    return common_strtod_l<double>(string, end_ptr, locale);
}

extern "C" long double __cdecl strtold(
    char const* const string,
    char**      const end_ptr
    )
{
    return common_strtod_l<double>(string, end_ptr, nullptr);
}

extern "C" long double __cdecl _strtold_l(
    char const* const string,
    char**      const end_ptr,
    _locale_t   const locale
    )
{
    return common_strtod_l<double>(string, end_ptr, locale);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Wide Strings => Floating Point
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" float __cdecl wcstof(
    wchar_t const* const string,
    wchar_t**      const end_ptr
    )
{
    return common_strtod_l<float>(string, end_ptr, nullptr);
}

extern "C" float __cdecl _wcstof_l(
    wchar_t const* const string,
    wchar_t**      const end_ptr,
    _locale_t      const locale
    )
{
    return common_strtod_l<float>(string, end_ptr, locale);
}

extern "C" double __cdecl wcstod(
    wchar_t const* const string,
    wchar_t**      const end_ptr
    )
{
    return common_strtod_l<double>(string, end_ptr, nullptr);
}

extern "C" double __cdecl _wcstod_l(
    wchar_t const* const string,
    wchar_t**      const end_ptr,
    _locale_t      const locale
    )
{
    return common_strtod_l<double>(string, end_ptr, locale);
}

extern "C" long double __cdecl wcstold(
    wchar_t const* const string,
    wchar_t**      const end_ptr
    )
{
    return common_strtod_l<double>(string, end_ptr, nullptr);
}

extern "C" long double __cdecl _wcstold_l(
    wchar_t const* const string,
    wchar_t**      const end_ptr,
    _locale_t      const locale
    )
{
    return common_strtod_l<double>(string, end_ptr, locale);
}
