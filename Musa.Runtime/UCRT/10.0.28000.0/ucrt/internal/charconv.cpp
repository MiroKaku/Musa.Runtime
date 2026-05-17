//
// charconv.cpp -- Kernel-mode character conversion helpers
//
// Only supports CP_ACP, CP_UTF8. Other code pages delegate to Musa.Core.
//
// Symbols provided by this overlay:
//   __acrt_MultiByteToWideChar, __acrt_WideCharToMultiByte
//   return_illegal_sequence, reset_and_return
//   __mblen_utf8, __c16rtomb_utf8, __c32rtomb_utf8
//   __mbsrtowcs_utf8, _putwch_nolock, c32rtomb
//   _wctomb_internal, _mbtowc_internal (kernel-mode ASCII-only)
//

#include <corecrt_internal.h>
#include <corecrt_internal_mbstring.h>
#include <corecrt_internal_ptd_propagation.h>
#include <stdint.h>

extern "C" int __cdecl __acrt_MultiByteToWideChar(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr,
    int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    if (CodePage != CP_ACP && CodePage != CP_UTF8) { return 0; }
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

namespace __crt_mbstring
{
    size_t return_illegal_sequence(mbstate_t* ps, __crt_cached_ptd_host& ptd)
    {
        *ps = {};
        ptd.get_errno().set(EILSEQ);
        return INVALID;
    }

    size_t reset_and_return(size_t retval, mbstate_t* ps)
    {
        *ps = {};
        return retval;
    }

    int __cdecl __mblen_utf8(char const* s)
    {
        if (!s || !*s) return 0;
        unsigned char c = static_cast<unsigned char>(*s);
        if (c < 0x80)       return 1;
        if (c < 0xC2)       return 1;
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

    size_t __cdecl __c32rtomb_utf8(char* s, char32_t c32, mbstate_t* ps, __crt_cached_ptd_host& ptd)
    {
        if (!s) { *ps = {}; return 1; }
        if (c32 == U'\0') { *s = '\0'; *ps = {}; return 1; }
        if ((c32 & ~0x7f) == 0) { *s = static_cast<char>(c32); return 1; }
        size_t trail_bytes;
        uint8_t lead_byte;
        if ((c32 & ~0x7ff) == 0) { trail_bytes = 1; lead_byte = 0xc0; }
        else if ((c32 & ~0xffff) == 0) {
            if (0xd800 <= c32 && c32 <= 0xdfff) return return_illegal_sequence(ps, ptd);
            trail_bytes = 2; lead_byte = 0xe0;
        }
        else if ((c32 & ~0x001fffff) == 0) {
            if (0x10ffff < c32) return return_illegal_sequence(ps, ptd);
            trail_bytes = 3; lead_byte = 0xf0;
        }
        else { return return_illegal_sequence(ps, ptd); }
        for (size_t i = trail_bytes; i > 0; --i) { s[i] = static_cast<char>((c32 & 0x3f) | 0x80); c32 >>= 6; }
        s[0] = static_cast<char>(c32) | lead_byte;
        return reset_and_return(trail_bytes + 1, ps);
    }

    size_t __cdecl __mbsrtowcs_utf8(
        wchar_t* dst, const char** src, size_t len, mbstate_t* ps, __crt_cached_ptd_host& ptd)
    {
        UNREFERENCED_PARAMETER(ps); UNREFERENCED_PARAMETER(ptd);
        if (!src || !*src) return 0;
        const char* s = *src;
        size_t count = 0;
        // Sizing pass when dst is null: count chars without writing
        while (*s) {
            if (dst && count >= len) break;
            unsigned char c = static_cast<unsigned char>(*s);
            wchar_t wc;
            size_t consumed;
            if (c < 0x80) {
                wc = static_cast<wchar_t>(c);
                consumed = 1;
            } else if (c < 0xC2 || c > 0xF4) {
                return static_cast<size_t>(-1); // invalid lead byte
            } else if (c < 0xE0) {
                if ((static_cast<unsigned char>(s[1]) & 0xC0) != 0x80)
                    return static_cast<size_t>(-1);
                wc = static_cast<wchar_t>(((c & 0x1F) << 6) |
                                          (static_cast<unsigned char>(s[1]) & 0x3F));
                consumed = 2;
            } else if (c < 0xF0) {
                if ((static_cast<unsigned char>(s[1]) & 0xC0) != 0x80 ||
                    (static_cast<unsigned char>(s[2]) & 0xC0) != 0x80)
                    return static_cast<size_t>(-1);
                wc = static_cast<wchar_t>(((c & 0x0F) << 12) |
                                          ((static_cast<unsigned char>(s[1]) & 0x3F) << 6) |
                                          (static_cast<unsigned char>(s[2]) & 0x3F));
                consumed = 3;
            } else {
                // 4-byte UTF-8 produces surrogate pair; not single wchar_t
                return static_cast<size_t>(-1);
            }
            if (dst) dst[count] = wc;
            ++count;
            s += consumed;
        }
        if (*s == '\0') {
            // Reached end of source: write null if dst has room, set src to nullptr
            if (dst) {
                if (count < len) dst[count] = L'\0';
                *src = nullptr;
            }
        } else {
            if (dst) *src = s;
        }
        return count;
    }
}

extern "C" void __cdecl _putwch_nolock(wchar_t) { }

// Kernel-mode simplified implementations (ASCII-only)
extern "C" errno_t __cdecl _wctomb_internal(
    int* _SizeConverted, char* _MbCh, size_t _SizeInBytes,
    wchar_t _WCh, __crt_cached_ptd_host& _Ptd)
{
    UNREFERENCED_PARAMETER(_Ptd);
    if (!_MbCh || _SizeInBytes == 0) return EINVAL;
    *_MbCh = static_cast<char>(_WCh & 0xFF);
    if (_SizeConverted) *_SizeConverted = 1;
    return 0;
}

extern "C" int __cdecl _mbtowc_internal(
    wchar_t* _DstCh, char const* _SrcCh, size_t _SrcSizeInBytes,
    __crt_cached_ptd_host& _Ptd)
{
    UNREFERENCED_PARAMETER(_SrcSizeInBytes); UNREFERENCED_PARAMETER(_Ptd);
    if (!_SrcCh || !*_SrcCh) return 0;
    if (_DstCh) *_DstCh = static_cast<wchar_t>(static_cast<unsigned char>(*_SrcCh));
    return 1;
}

extern "C" size_t __cdecl c32rtomb(char* s, char32_t c32, mbstate_t* ps)
{
    __crt_cached_ptd_host ptd;
    return __crt_mbstring::__c32rtomb_utf8(s, c32, ps, ptd);
}

// __mbrtowc_utf8 -- UTF-8 multibyte to wide char conversion
namespace __crt_mbstring
{
    size_t __cdecl __mbrtowc_utf8(
        wchar_t* dst, const char* src, size_t len, mbstate_t* ps, __crt_cached_ptd_host& ptd)
    {
        UNREFERENCED_PARAMETER(ps); UNREFERENCED_PARAMETER(ptd);
        if (!src || !*src) { if (dst) *dst = 0; return 0; }
        unsigned char c = static_cast<unsigned char>(*src);
        if (c < 0x80) { if (dst) *dst = static_cast<wchar_t>(c); return 1; }
        if (c < 0xC2 || c > 0xF4) return static_cast<size_t>(-1); // invalid
        if (c < 0xE0) {
            if (len < 2 || (src[1] & 0xC0) != 0x80) return static_cast<size_t>(-1);
            if (dst) *dst = static_cast<wchar_t>(((c & 0x1F) << 6) | (src[1] & 0x3F));
            return 2;
        }
        if (c < 0xF0) {
            if (len < 3 || (src[1] & 0xC0) != 0x80 || (src[2] & 0xC0) != 0x80) return static_cast<size_t>(-1);
            if (dst) *dst = static_cast<wchar_t>(((c & 0x0F) << 12) | ((src[1] & 0x3F) << 6) | (src[2] & 0x3F));
            return 3;
        }
        // 4-byte sequence -> surrogate pair (not representable in single wchar_t)
        if (len < 4) return static_cast<size_t>(-2); // incomplete
        return static_cast<size_t>(-1);
    }
}

// __ascii_wcsicmp -- ASCII-only case-insensitive wide string comparison
extern "C" int __cdecl __ascii_wcsicmp(const wchar_t* s1, const wchar_t* s2)
{
    if (!s1 || !s2) return -1;
    wchar_t c1, c2;
    do {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 >= L'A' && c1 <= L'Z') c1 += L'a' - L'A';
        if (c2 >= L'A' && c2 <= L'Z') c2 += L'a' - L'A';
    } while (c1 && c1 == c2);
    return c1 - c2;
}
