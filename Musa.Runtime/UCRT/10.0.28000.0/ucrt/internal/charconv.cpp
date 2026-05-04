//
// charconv.cpp -- Kernel-mode character conversion helpers
//
// Only supports CP_ACP, CP_UTF8. Other code pages delegate to Musa.Core.
//
// Summary of symbol sourcing:
//   Musa.Core thunks:  MultiByteToWideChar, WideCharToMultiByte (kernel32 → kernel-mode safe)
//   this overlay:      __acrt_MultiByteToWideChar (→ MultiByteToWideChar, CP_ACP/CP_UTF8 only)
//                      __acrt_WideCharToMultiByte (→ WideCharToMultiByte, CP_ACP/CP_UTF8 only)
//                      _mbtowc_internal (simple ASCII, no MBCS)
//                      __mbsrtowcs_utf8 (ASCII-only in kernel mode)
//                      _putwch_nolock (no-op stub)
//   Code page scope:   CP_ACP (0) and CP_UTF8 (65001) only.
//                      UTF-16 is implicit (native wchar_t encoding, no conversion needed).
//                      All other code pages delegate to Musa.Core.
//   ntoskrnl:          mbstowcs, wcstombs (used via base SDK, not this file)
//
// Only handles CP_ACP and CP_UTF8. Other code pages return 0 and delegate to Musa.Core.

#include <corecrt_internal.h>

extern "C" int __cdecl __acrt_MultiByteToWideChar(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr,
    int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    if (CodePage != CP_ACP && CodePage != CP_UTF8) { return 0; /* delegate to Musa.Core for unsupported code pages */ }
    return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

extern "C" int __cdecl __acrt_WideCharToMultiByte(
    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr,
    int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
    LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    if (CodePage != CP_ACP && CodePage != CP_UTF8) { return 0; }
    return WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}
// _mbtowc_internal -- simple single-byte to wchar_t conversion (ASCII subset).
// Code page scope: assumes C/POSIX locale semantics; no MBCS handling.
// For non-ASCII multibyte sequences, use Musa.Core with explicit code page support.
extern "C" int __cdecl _mbtowc_internal(wchar_t* pwc, char const* s, size_t n, struct _Mbstatet* pst)
{
    UNREFERENCED_PARAMETER(n); UNREFERENCED_PARAMETER(pst);
    if (!s || !*s) return 0;
    if (pwc) *pwc = static_cast<wchar_t>(static_cast<unsigned char>(*s));
    return 1;
}

namespace __crt_mbstring
{
    // __mbsrtowcs_utf8 -- byte-to-wchar_t conversion treating input as raw bytes (ASCII-safe subset).
    // Code page scope: does NOT perform full UTF-8 decoding; each byte maps directly to its wchar_t value.
    // For proper UTF-8 multi-byte sequence handling, delegate to Musa.Core.
    size_t __cdecl __mbsrtowcs_utf8(
        wchar_t* dst, const char** src, size_t len, mbstate_t* ps, __crt_cached_ptd_host& ptd)
    {
        UNREFERENCED_PARAMETER(ps); UNREFERENCED_PARAMETER(ptd);
        if (!src || !*src) return 0;
        const char* s = *src;
        size_t count = 0;
        while (*s && count < len) { if (dst) dst[count] = static_cast<wchar_t>(static_cast<unsigned char>(*s)); ++s; ++count; }
        *src = s;
        return count;
    }
}

extern "C" void __cdecl _putwch_nolock(wchar_t) { }

// ---- printf engine dependencies ----

// _wctomb_internal — wchar_t to multibyte (printf %lc/%C support)
extern "C" int __cdecl _wctomb_internal(char* s, wchar_t wchar, struct _Mbstatet* pst, __crt_cached_ptd_host const& ptd)
{
    UNREFERENCED_PARAMETER(pst); UNREFERENCED_PARAMETER(ptd);
    if (!s) return 0;
    *s = static_cast<char>(wchar & 0xFF);
    return 1;
}

namespace __crt_mbstring
{
    int __cdecl __mblen_utf8(char const* s)
    {
        if (!s || !*s) return 0;
        unsigned char c = static_cast<unsigned char>(*s);
        if (c < 0x80)       return 1;
        if (c < 0xC2)       return 1; // invalid, treat as single byte
        if (c < 0xE0)       return 2;
        if (c < 0xF0)       return 3;
        return 4;
    }

    unsigned __int64 __cdecl __c16rtomb_utf8(
        char* dst, char16_t c16, struct _Mbstatet* pst, __crt_cached_ptd_host& ptd)
    {
        UNREFERENCED_PARAMETER(pst); UNREFERENCED_PARAMETER(ptd);
        if (!dst) return 0;
        if (c16 < 0x80) { dst[0] = static_cast<char>(c16); return 1; }
        if (c16 < 0x800) { dst[0] = static_cast<char>(0xC0 | (c16 >> 6)); dst[1] = static_cast<char>(0x80 | (c16 & 0x3F)); return 2; }
        dst[0] = static_cast<char>(0xE0 | (c16 >> 12)); dst[1] = static_cast<char>(0x80 | ((c16 >> 6) & 0x3F)); dst[2] = static_cast<char>(0x80 | (c16 & 0x3F));
        return 3;
    }
}

