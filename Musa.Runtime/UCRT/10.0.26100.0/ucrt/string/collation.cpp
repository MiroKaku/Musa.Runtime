//
// collation.cpp -- Kernel-mode string collation helpers
//
// Provides kernel-mode implementations of locale-dependent string collation
// functions using simple case-insensitive comparison.
//
//
// Self-contained kernel-mode implementation — no ntoskrnl dependency.
// Provides _strnicoll and _wcsnicoll for collation in kernel mode.
//
#include <corecrt_internal.h>
#include <string.h>
#include <wchar.h>

// _strnicoll -- from base SDK env/getenv.cpp, env/setenv.cpp
extern "C" int __cdecl _strnicoll(const char* s1, const char* s2, size_t n)
{
    return _strnicmp(s1, s2, n);
}

// _wcsnicoll -- from base SDK env/getenv.cpp, env/setenv.cpp
extern "C" int __cdecl _wcsnicoll(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    return _wcsnicmp(s1, s2, n);
}
