//
// wsetlocale.cpp - Kernel-mode locale: C locale only (CP_ACP/UTF-8)
//
// Minimal implementation for kernel mode where only the "C" locale is supported.
// Queries always return "C"/L"C"; attempts to set any other locale fail.
//

#include <corecrt_internal.h>
#include <locale.h>

extern "C" {

// Global flag: locale has NOT been changed from C
long __acrt_locale_changed_data = 0;

// Forward from nlsdata.cpp
extern wchar_t __acrt_wide_c_locale_string[];
extern __crt_locale_data __acrt_initial_locale_data;
extern __crt_locale_pointers __acrt_initial_locale_pointers;

// setlocale — ANSI version, delegates to _wsetlocale
char* __cdecl setlocale(int category, char const* locale)
{
    UNREFERENCED_PARAMETER(category);

    if (locale == nullptr || *locale == '\0' || strcmp(locale, "C") == 0) {
        return const_cast<char*>("C");
    }
    return nullptr;
}

// _wsetlocale — wide-char version, used by STL locale
wchar_t* __cdecl _wsetlocale(int category, wchar_t const* wlocale)
{
    UNREFERENCED_PARAMETER(category);

    if (wlocale == nullptr || *wlocale == L'\0' || wcscmp(wlocale, L"C") == 0) {
        return __acrt_wide_c_locale_string;
    }
    return nullptr;
}

// _wcreate_locale — creates a _locale_t from a name, C locale only
_locale_t __cdecl _wcreate_locale(int category, wchar_t const* wlocale)
{
    UNREFERENCED_PARAMETER(category);

    if (wlocale != nullptr && *wlocale != L'\0' && wcscmp(wlocale, L"C") != 0) {
        return nullptr;
    }
    // _locale_t points to initial locale pointers
    return static_cast<_locale_t>(&__acrt_initial_locale_pointers);
}

// _create_locale — ANSI version
_locale_t __cdecl _create_locale(int category, char const* locale)
{
    if (locale == nullptr || strcmp(locale, "C") != 0) return nullptr;
    return _wcreate_locale(category, L"C");
}

// _free_locale — no-op in kernel mode
void __cdecl _free_locale(_locale_t locale)
{
    UNREFERENCED_PARAMETER(locale);
}

// __acrt_eagerly_load_locale_apis — no-op (no dynamic loading)
void __cdecl __acrt_eagerly_load_locale_apis()
{
}

// __acrt_update_thread_locale_data — returns initial locale
__crt_locale_data* __cdecl __acrt_update_thread_locale_data()
{
    return &__acrt_initial_locale_data;
}

// __acrt_get_global_locale — returns initial locale
__crt_locale_data* __cdecl __acrt_get_global_locale()
{
    return &__acrt_initial_locale_data;
}

// __acrt_set_locale_changed — no-op
void __cdecl __acrt_set_locale_changed()
{
}

// _updatetlocinfoEx_nolock — copy locale info pointer
__crt_locale_data* __cdecl _updatetlocinfoEx_nolock(
    __crt_locale_data** dst, __crt_locale_data* src)
{
    if (dst) *dst = src;
    return src;
}

// __acrt_release_locale_ref — no-op
void __cdecl __acrt_release_locale_ref(__crt_locale_data*)
{
}

// __acrt_free_locale — no-op
void __cdecl __acrt_free_locale(__crt_locale_data*)
{
}

// sync_legacy_variables_lk — no-op
void __cdecl sync_legacy_variables_lk()
{
}

} // extern "C"
