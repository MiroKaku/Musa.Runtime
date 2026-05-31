//
// StlLCMapStringA.cpp - kernel overlay: ASCII-only LCMapString for C locale
//
#include <string.h>
#include <limits.h>

extern "C" int __cdecl ___lc_collate_cp_func();

extern "C" int __cdecl __crtLCMapStringA(
    const wchar_t* LocaleName,
    unsigned long  dwMapFlags,
    const char*    lpSrcStr,
    int            cchSrc,
    char*          lpDestStr,
    int            cchDest,
    int            code_page,
    int            bError) noexcept
{
    (void)LocaleName;
    (void)code_page;
    (void)bError;

    // C locale: only LCMAP_LOWERCASE/UPPERCASE/SORTKEY needed
    // LCMAP_SORTKEY used by xstrxfrm/regex for collation
    #define LCMAP_LOWERCASE 0x00000100
    #define LCMAP_UPPERCASE 0x00000200
    #define LCMAP_SORTKEY  0x00000400

    // Find source length (handle null-terminated or bounded)
    int srcLen = cchSrc;
    if (srcLen < 0) {
        srcLen = (int)strlen(lpSrcStr) + 1;
    } else if (srcLen > 0) {
        int cnt = 0;
        while (cnt < srcLen && lpSrcStr[cnt] != '\0') ++cnt;
        if (cnt < srcLen) srcLen = cnt + 1;
    }

    if (dwMapFlags & LCMAP_SORTKEY) {
        // C locale: sort key = identity
        if (cchDest == 0)
            return srcLen; // return required size
        int copyLen = srcLen < cchDest ? srcLen : cchDest;
        memcpy(lpDestStr, lpSrcStr, copyLen);
        return copyLen;
    }

    // Case mapping: ASCII only
    if (cchDest == 0)
        return srcLen;

    int i = 0;
    if (dwMapFlags & LCMAP_LOWERCASE) {
        for (; i < srcLen && i < cchDest; ++i) {
            unsigned char c = (unsigned char)lpSrcStr[i];
            if (c >= 'A' && c <= 'Z')
                lpDestStr[i] = (char)(c + ('a' - 'A'));
            else
                lpDestStr[i] = (char)c;
        }
    } else if (dwMapFlags & LCMAP_UPPERCASE) {
        for (; i < srcLen && i < cchDest; ++i) {
            unsigned char c = (unsigned char)lpSrcStr[i];
            if (c >= 'a' && c <= 'z')
                lpDestStr[i] = (char)(c - ('a' - 'A'));
            else
                lpDestStr[i] = (char)c;
        }
    } else {
        // unknown flags: identity copy
        for (; i < srcLen && i < cchDest; ++i)
            lpDestStr[i] = lpSrcStr[i];
    }
    return i;
}

// Provide ___lc_collate_cp_func for C locale
extern "C" int __cdecl ___lc_collate_cp_func()
{
    return 65001; // CP_UTF8
}

// Provide ___lc_locale_name_func for C locale (all locale names are nullptr)
static wchar_t* c_locale_names[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
extern "C" wchar_t** __cdecl ___lc_locale_name_func()
{
    return c_locale_names;
}

// Stub _Atexit for kernel mode (STL internal)
void __cdecl _Atexit(void (__cdecl* pf)())
{
    (void)pf;
}

// Provide __crtCompareStringA for C locale (ASCII memcmp)
extern "C" int __cdecl __crtCompareStringA(
    const wchar_t* LocaleName,
    unsigned long  dwCmpFlags,
    const char*    lpString1,
    int            cchCount1,
    const char*    lpString2,
    int            cchCount2,
    int            code_page) noexcept
{
    (void)LocaleName;
    (void)dwCmpFlags;
    (void)code_page;

    if (cchCount1 < 0 || cchCount2 < 0) return 0;
    int len1 = cchCount1;
    int len2 = cchCount2;

    // Find actual lengths (stop at null terminator if present)
    if (len1 > 0) { int n = 0; while (n < len1 && lpString1[n]) ++n; if (n < len1) len1 = n + 1; }
    if (len2 > 0) { int n = 0; while (n < len2 && lpString2[n]) ++n; if (n < len2) len2 = n + 1; }

    int cmpLen = len1 < len2 ? len1 : len2;
    int r = memcmp(lpString1, lpString2, cmpLen);
    if (r != 0) return r ? (r > 0 ? 1 /* CSTR_GREATER_THAN */ : -1 /* CSTR_LESS_THAN */) : 0;
    if (len1 < len2) return -1; // CSTR_LESS_THAN
    if (len1 > len2) return 1;  // CSTR_GREATER_THAN
    return 2; // CSTR_EQUAL (2 means strings are equal)
}

// Provide ___mb_cur_max_func for CP_UTF8 (max 4 bytes)
extern "C" int __cdecl ___mb_cur_max_func()
{
    return 4; // CP_UTF8 max bytes
}
