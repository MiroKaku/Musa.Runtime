// Kernel-mode minimal locale data -- C locale only.
//
// nlsdata.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Globals for the locale and code page implementation.  These are utilized by
// almost all locale-dependent functions.
// In kernel mode, only the C locale is provided.

#include <corecrt_internal.h>
#include <locale.h>
#include <limits.h>


// ---- Kernel Mode ----

// Forward-declared from _ctype.cpp
extern "C" const unsigned short _pctype_data[];

// __acrt_initial_multibyte_data is defined in mbctype.cpp
extern "C" __crt_multibyte_data __acrt_initial_multibyte_data;

__crt_state_management::dual_state_global<__crt_locale_data*> __acrt_current_locale_data;

extern "C" wchar_t __acrt_wide_c_locale_string[] { L"C" };

// C locale lconv data
extern "C" char    __acrt_lconv_static_decimal  []{"."};
extern "C" char    __acrt_lconv_static_null     []{""};
extern "C" wchar_t __acrt_lconv_static_W_decimal[]{L"."};
extern "C" wchar_t __acrt_lconv_static_W_null   []{L""};

extern "C" struct lconv __acrt_lconv_c
{
    __acrt_lconv_static_decimal,   // decimal_point
    __acrt_lconv_static_null,      // thousands_sep
    __acrt_lconv_static_null,      // grouping
    __acrt_lconv_static_null,      // int_curr_symbol
    __acrt_lconv_static_null,      // currency_symbol
    __acrt_lconv_static_null,      // mon_decimal_point
    __acrt_lconv_static_null,      // mon_thousands_sep
    __acrt_lconv_static_null,      // mon_grouping
    __acrt_lconv_static_null,      // positive_sign
    __acrt_lconv_static_null,      // negative_sign
    CHAR_MAX,                      // int_frac_digits
    CHAR_MAX,                      // frac_digits
    CHAR_MAX,                      // p_cs_precedes
    CHAR_MAX,                      // p_sep_by_space
    CHAR_MAX,                      // n_cs_precedes
    CHAR_MAX,                      // n_sep_by_space
    CHAR_MAX,                      // p_sign_posn
    CHAR_MAX,                      // n_sign_posn
    __acrt_lconv_static_W_decimal, // _W_decimal_point
    __acrt_lconv_static_W_null,    // _W_thousands_sep
    __acrt_lconv_static_W_null,    // _W_int_curr_symbol
    __acrt_lconv_static_W_null,    // _W_currency_symbol
    __acrt_lconv_static_W_null,    // _W_mon_decimal_point
    __acrt_lconv_static_W_null,    // _W_mon_thousands_sep
    __acrt_lconv_static_W_null,    // _W_positive_sign
    __acrt_lconv_static_W_null,    // _W_negative_sign
};

extern "C" __crt_lc_time_data const __lc_time_c
{
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" },
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
    { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" },
    { "AM", "PM" },
    "MM/dd/yy",
    "dddd, MMMM dd, yyyy",
    "HH:mm:ss",
    1, 0,
    { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" },
    { L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday" },
    { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" },
    { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" },
    { L"AM", L"PM" },
    L"MM/dd/yy",
    L"dddd, MMMM dd, yyyy",
    L"HH:mm:ss",
    L"en-US"
};



extern "C" __crt_locale_data __acrt_initial_locale_data
{
    { const_cast<unsigned short*>(_pctype_data + 1), 1, CP_UTF8 },
    1, CP_UTF8, CP_UTF8, 1,
    { {nullptr}, {nullptr}, {nullptr}, {nullptr}, {nullptr}, {nullptr} },
    nullptr, nullptr, nullptr, &__acrt_lconv_c,
    nullptr, nullptr,
    nullptr, nullptr,
    const_cast<__crt_lc_time_data*>(&__lc_time_c),
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

extern "C" __crt_locale_pointers __acrt_initial_locale_pointers
{
    &__acrt_initial_locale_data,
    &__acrt_initial_multibyte_data
};

extern "C" unsigned int __cdecl ___lc_codepage_func()
{
    return CP_UTF8;
}
