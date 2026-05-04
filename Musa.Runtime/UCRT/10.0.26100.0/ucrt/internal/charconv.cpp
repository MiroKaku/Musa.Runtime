//
// charconv.cpp -- Kernel-mode character conversion helpers
//
#include <corecrt_internal.h>
#include <ntrtl.h>

extern "C" int __cdecl __acrt_MultiByteToWideChar(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr,
    int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    UNREFERENCED_PARAMETER(dwFlags);
    if (!lpMultiByteStr) return 0;

    ULONG cbSrc = (cbMultiByte == -1) ? static_cast<ULONG>(strlen(lpMultiByteStr)) + 1 : static_cast<ULONG>(cbMultiByte);
    ULONG cchDst = (cchWideChar > 0) ? static_cast<ULONG>(cchWideChar) : 0;
    if (cbSrc == 0) return 0;

    NTSTATUS Status = (CodePage == CP_UTF8 || CodePage == CP_ACP)
        ? RtlUTF8ToUnicodeN(lpWideCharStr, cchDst * sizeof(WCHAR), nullptr, lpMultiByteStr, cbSrc)
        : RtlMultiByteToUnicodeN(lpWideCharStr, cchDst * sizeof(WCHAR), nullptr, lpMultiByteStr, cbSrc);

    if (!NT_SUCCESS(Status)) { if (cchDst == 0) return static_cast<int>(cbSrc); return 0; }
    return static_cast<int>(cbSrc);
}

extern "C" int __cdecl __acrt_WideCharToMultiByte(
    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr,
    int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
    LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    UNREFERENCED_PARAMETER(dwFlags); UNREFERENCED_PARAMETER(lpDefaultChar);
    if (!lpWideCharStr) return 0;

    ULONG cchSrc = (cchWideChar == -1) ? static_cast<ULONG>(wcslen(lpWideCharStr)) + 1 : static_cast<ULONG>(cchWideChar);
    ULONG cbDst = (cbMultiByte > 0) ? static_cast<ULONG>(cbMultiByte) : 0;
    if (cchSrc == 0) { if (lpUsedDefaultChar) *lpUsedDefaultChar = FALSE; return 0; }

    NTSTATUS Status = (CodePage == CP_UTF8 || CodePage == CP_ACP)
        ? RtlUnicodeToUTF8N(lpMultiByteStr, cbDst, nullptr, lpWideCharStr, cchSrc * sizeof(WCHAR))
        : RtlUnicodeToMultiByteN(lpMultiByteStr, cbDst, nullptr, lpWideCharStr, cchSrc * sizeof(WCHAR));

    if (lpUsedDefaultChar) *lpUsedDefaultChar = FALSE;
    if (!NT_SUCCESS(Status)) { if (cbDst == 0) return static_cast<int>(cchSrc * 2); return 0; }
    return static_cast<int>(cchSrc);
}

extern "C" int __cdecl _mbtowc_internal(wchar_t* pwc, char const* s, size_t n, struct _Mbstatet* pst)
{
    UNREFERENCED_PARAMETER(n); UNREFERENCED_PARAMETER(pst);
    if (!s || !*s) return 0;
    if (pwc) *pwc = static_cast<wchar_t>(static_cast<unsigned char>(*s));
    return 1;
}

namespace __crt_mbstring
{
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
