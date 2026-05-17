//
// _ctype.cpp — Kernel-mode character classification (C locale only)
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Self-contained ASCII implementation — no ntoskrnl dependency.
// Supports only C locale (characters 0-255).
// All _l-suffix functions ignore locale and delegate to base functions.
//

#include <corecrt_internal.h>
#include <ctype.h>
#include <locale.h>

#pragma optimize("t", on)


extern "C" int (__cdecl _isalpha_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return isalpha(c);
}

extern "C" int (__cdecl isalpha)(int const c)
{
    return
        (c >= 0x41 && c <= 0x5a) ||
        (c >= 0x61 && c <= 0x7a);
}


extern "C" int (__cdecl _isupper_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return (c >= 0x41 && c <= 0x5a);
}

extern "C" int (__cdecl isupper)(int const c)
{
    return (c >= 0x41 && c <= 0x5a);
}


extern "C" int (__cdecl _islower_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return (c >= 0x61 && c <= 0x7a);
}

extern "C" int (__cdecl islower)(int const c)
{
    return (c >= 0x61 && c <= 0x7a);
}


extern "C" int (__cdecl _isdigit_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return (c >= 0x30 && c <= 0x39);
}

extern "C" int (__cdecl isdigit)(int const c)
{
    return (c >= 0x30 && c <= 0x39);
}


extern "C" int (__cdecl _isxdigit_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return ((c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x46) || (c >= 0x61 && c <= 0x66));
}

extern "C" int (__cdecl isxdigit)(int const c)
{
    return ((c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x46) || (c >= 0x61 && c <= 0x66));
}


extern "C" int (__cdecl _isspace_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return (c == ' ' || (c >= 0x09 && c <= 0x0D));
}

extern "C" int (__cdecl isspace)(int const c)
{
    return (c == ' ' || (c >= 0x09 && c <= 0x0D));
}


extern "C" int (__cdecl _isprint_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return (c >= 0x20 && c <= 0x7E);
}

extern "C" int (__cdecl isprint)(int const c)
{
    return (c >= 0x20 && c <= 0x7E);
}


extern "C" int (__cdecl _ispunct_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return ispunct(c);
}

extern "C" int (__cdecl ispunct)(int const c)
{
    return
        (c >= 0x21 && c <= 0x2f) ||
        (c >= 0x3a && c <= 0x40) ||
        (c >= 0x5b && c <= 0x60) ||
        (c >= 0x7b && c <= 0x7e);
}


extern "C" int (__cdecl _isblank_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return isblank(c);
}

extern "C" int (__cdecl isblank)(int const c)
{
    if (c == '\t') return _BLANK;
    if (c == ' ')  return _SPACE;
    return 0;
}


extern "C" int (__cdecl _isalnum_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return isalnum(c);
}

extern "C" int (__cdecl isalnum)(int const c)
{
    return isdigit(c) || isalpha(c);
}


extern "C" int (__cdecl _isgraph_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return isgraph(c);
}

extern "C" int (__cdecl isgraph)(int const c)
{
    return (c >= 0x21 && c <= 0x7E);
}


extern "C" int (__cdecl _iscntrl_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return iscntrl(c);
}

extern "C" int (__cdecl iscntrl)(int const c)
{
    return (c == 0x7f || (c >= 0x00 && c <= 0x1f));
}


extern "C" int (__cdecl __isascii)(int const c)
{
    return ((unsigned)(c) < 0x80);
}

extern "C" int (__cdecl __toascii)(int const c)
{
    return ((c) & 0x7f);
}


extern "C" int (__cdecl _iscsymf_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return __iscsymf(c);
}

extern "C" int (__cdecl __iscsymf)(int const c)
{
    return (isalpha(c) || ((c) == '_'));
}


extern "C" int (__cdecl _iscsym_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return __iscsym(c);
}

extern "C" int (__cdecl __iscsym)(int const c)
{
    return (isalnum(c) || ((c) == '_'));
}


extern "C" int (__cdecl toupper)(int const c)
{
    if (c >= 'a' && c <= 'z') return c - 0x20;
    return c;
}

extern "C" int (__cdecl _toupper_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return toupper(c);
}

extern "C" int (__cdecl tolower)(int const c)
{
    if (c >= 'A' && c <= 'Z') return c + 0x20;
    return c;
}

extern "C" int (__cdecl _tolower_l)(int const c, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return tolower(c);
}


extern "C" int __cdecl _isctype(int const c, int const mask)
{
    if (c >= -1 && c <= 255)
    {
        int result = 0;
        if (mask & _UPPER)   result |= (c >= 'A' && c <= 'Z');
        if (mask & _LOWER)   result |= (c >= 'a' && c <= 'z');
        if (mask & _DIGIT)   result |= (c >= '0' && c <= '9');
        if (mask & _SPACE)   result |= (c == ' ' || (c >= 0x09 && c <= 0x0D));
        if (mask & _PUNCT)   result |= ispunct(c);
        if (mask & _CONTROL) result |= (c == 0x7F || (c >= 0x00 && c <= 0x1F));
        if (mask & _BLANK)   result |= (c == ' ' || c == '\t');
        if (mask & _HEX)     result |= isxdigit(c);
        return result;
    }
    return 0;
}

extern "C" int __cdecl _isctype_l(int const c, int const mask, _locale_t const locale)
{
    UNREFERENCED_PARAMETER(locale);
    return _isctype(c, mask);
}

#if defined _DEBUG
extern "C" int __cdecl _chvalidator(int const c, int const mask)
{
    _ASSERTE(c >= -1 && c <= 255);
    return _isctype_l(c, mask, nullptr);
}

extern "C" int __cdecl _chvalidator_l(_locale_t const locale, int const c, int const mask)
{
    UNREFERENCED_PARAMETER(locale);
    _ASSERTE(c >= -1 && c <= 255);
    return _isctype_l(c, mask, locale);
}
#endif

// ---- C locale character type table (_pctype) for strtox/strtod ----
//
// Layout: _pctype_data[c + 1] gives the ctype mask for character c.
// _pctype_data[0] is the EOF (-1) entry. __pctype_func() returns
// _pctype_data + 1 so that _pctype[c] indexes correctly for c in [-1,255].

extern "C" const unsigned short _pctype_data[257] = {
    /* [0]  -1 EOF */ 0,
    /* [1]  00 NUL */ _CONTROL,           /* [2]  01 SOH */ _CONTROL,
    /* [3]  02 STX */ _CONTROL,           /* [4]  03 ETX */ _CONTROL,
    /* [5]  04 EOT */ _CONTROL,           /* [6]  05 ENQ */ _CONTROL,
    /* [7]  06 ACK */ _CONTROL,           /* [8]  07 BEL */ _CONTROL,
    /* [9]  08 BS  */ _CONTROL,
    /* [10] 09 HT  */ _SPACE | _CONTROL,  /* [11] 0A LF  */ _SPACE | _CONTROL,
    /* [12] 0B VT  */ _SPACE | _CONTROL,  /* [13] 0C FF  */ _SPACE | _CONTROL,
    /* [14] 0D CR  */ _SPACE | _CONTROL,
    /* [15] 0E SI  */ _CONTROL,           /* [16] 0F SO  */ _CONTROL,
    /* [17] 10 DLE */ _CONTROL,           /* [18] 11 DC1 */ _CONTROL,
    /* [19] 12 DC2 */ _CONTROL,           /* [20] 13 DC3 */ _CONTROL,
    /* [21] 14 DC4 */ _CONTROL,           /* [22] 15 NAK */ _CONTROL,
    /* [23] 16 SYN */ _CONTROL,           /* [24] 17 ETB */ _CONTROL,
    /* [25] 18 CAN */ _CONTROL,           /* [26] 19 EM  */ _CONTROL,
    /* [27] 1A SUB */ _CONTROL,           /* [28] 1B ESC */ _CONTROL,
    /* [29] 1C FS  */ _CONTROL,           /* [30] 1D GS  */ _CONTROL,
    /* [31] 1E RS  */ _CONTROL,           /* [32] 1F US  */ _CONTROL,
    /* [33] 20 ' ' */ _SPACE | _BLANK,
    /* [34] 21 !   */ _PUNCT, /* [35] 22 "   */ _PUNCT, /* [36] 23 #   */ _PUNCT,
    /* [37] 24 $   */ _PUNCT, /* [38] 25 %   */ _PUNCT, /* [39] 26 &   */ _PUNCT,
    /* [40] 27 '   */ _PUNCT, /* [41] 28 (   */ _PUNCT, /* [42] 29 )   */ _PUNCT,
    /* [43] 2A *   */ _PUNCT, /* [44] 2B +   */ _PUNCT, /* [45] 2C ,   */ _PUNCT,
    /* [46] 2D -   */ _PUNCT, /* [47] 2E .   */ _PUNCT, /* [48] 2F /   */ _PUNCT,
    /* [49] 30 0   */ _DIGIT | _HEX, /* [50] 31 1 */ _DIGIT | _HEX,
    /* [51] 32 2   */ _DIGIT | _HEX, /* [52] 33 3 */ _DIGIT | _HEX,
    /* [53] 34 4   */ _DIGIT | _HEX, /* [54] 35 5 */ _DIGIT | _HEX,
    /* [55] 36 6   */ _DIGIT | _HEX, /* [56] 37 7 */ _DIGIT | _HEX,
    /* [57] 38 8   */ _DIGIT | _HEX, /* [58] 39 9 */ _DIGIT | _HEX,
    /* [59] 3A :   */ _PUNCT, /* [60] 3B ;   */ _PUNCT, /* [61] 3C <   */ _PUNCT,
    /* [62] 3D =   */ _PUNCT, /* [63] 3E >   */ _PUNCT, /* [64] 3F ?   */ _PUNCT,
    /* [65] 40 @   */ _PUNCT,
    /* [66] 41 A   */ _UPPER | _HEX, /* [67] 42 B */ _UPPER | _HEX,
    /* [68] 43 C   */ _UPPER | _HEX, /* [69] 44 D */ _UPPER | _HEX,
    /* [70] 45 E   */ _UPPER | _HEX, /* [71] 46 F */ _UPPER | _HEX,
    /* [72] 47 G   */ _UPPER, /* [73] 48 H */ _UPPER, /* [74] 49 I */ _UPPER,
    /* [75] 4A J   */ _UPPER, /* [76] 4B K */ _UPPER, /* [77] 4C L */ _UPPER,
    /* [78] 4D M   */ _UPPER, /* [79] 4E N */ _UPPER, /* [80] 4F O */ _UPPER,
    /* [81] 50 P   */ _UPPER, /* [82] 51 Q */ _UPPER, /* [83] 52 R */ _UPPER,
    /* [84] 53 S   */ _UPPER, /* [85] 54 T */ _UPPER, /* [86] 55 U */ _UPPER,
    /* [87] 56 V   */ _UPPER, /* [88] 57 W */ _UPPER, /* [89] 58 X */ _UPPER,
    /* [90] 59 Y   */ _UPPER, /* [91] 5A Z */ _UPPER,
    /* [92] 5B [   */ _PUNCT, /* [93] 5C \ */ _PUNCT, /* [94] 5D ]   */ _PUNCT,
    /* [95] 5E ^   */ _PUNCT, /* [96] 5F _ */ _PUNCT, /* [97] 60 `   */ _PUNCT,
    /* [98] 61 a   */ _LOWER | _HEX, /* [99] 62 b */ _LOWER | _HEX,
    /* [100]63 c   */ _LOWER | _HEX, /* [101]64 d */ _LOWER | _HEX,
    /* [102]65 e   */ _LOWER | _HEX, /* [103]66 f */ _LOWER | _HEX,
    /* [104]67 g   */ _LOWER, /* [105]68 h */ _LOWER, /* [106]69 i */ _LOWER,
    /* [107]6A j   */ _LOWER, /* [108]6B k */ _LOWER, /* [109]6C l */ _LOWER,
    /* [110]6D m   */ _LOWER, /* [111]6E n */ _LOWER, /* [112]6F o */ _LOWER,
    /* [113]70 p   */ _LOWER, /* [114]71 q */ _LOWER, /* [115]72 r */ _LOWER,
    /* [116]73 s   */ _LOWER, /* [117]74 t */ _LOWER, /* [118]75 u */ _LOWER,
    /* [119]76 v   */ _LOWER, /* [120]77 w */ _LOWER, /* [121]78 x */ _LOWER,
    /* [122]79 y   */ _LOWER, /* [123]7A z */ _LOWER,
    /* [124]7B {   */ _PUNCT, /* [125]7C | */ _PUNCT,
    /* [126]7D }   */ _PUNCT, /* [127]7E ~ */ _PUNCT,
    /* [128]7F DEL */ _CONTROL,
    /* [129..256] 80..FF: zero-initialized by aggregate init */
};

extern "C" const unsigned short* __cdecl __pctype_func()
{
    return _pctype_data + 1;
}

extern "C" int __cdecl iswctype(wint_t c, wctype_t type)
{
    if (c == WEOF) return 0;
    if (c < 0 || c > 255) return 0;
    return (_pctype_data[c + 1] & type) != 0;
}

// _ismbblead — from base SDK filesystem/splitpath.cpp
extern "C" int __cdecl _ismbblead(unsigned int c)
{
    UNREFERENCED_PARAMETER(c);
    return 0;
}

// _mbsdec — from base SDK filesystem/makepath.cpp
extern "C" unsigned char* __cdecl _mbsdec(const unsigned char* start, const unsigned char* current)
{
    if (current > start)
        return const_cast<unsigned char*>(current - 1);
    return const_cast<unsigned char*>(start);
}

// _mbtowc_l — multibyte to wide char with locale (kernel mode: ignore locale)
extern "C" int __cdecl _mbtowc_l(wchar_t* pwc, const char* s, size_t n, _locale_t locale)
{
    UNREFERENCED_PARAMETER(locale);
    UNREFERENCED_PARAMETER(n);
    if (!s || !*s) return 0;
    if (pwc) *pwc = static_cast<wchar_t>(static_cast<unsigned char>(*s));
    return 1;
}

// wctomb_s — wide char to multibyte (secure, kernel mode: simple ASCII)
extern "C" errno_t __cdecl wctomb_s(int* pRetValue, char* mbchar, size_t sizeInBytes, wchar_t wchar)
{
    if (!pRetValue) return EINVAL;
    if (!mbchar || sizeInBytes < 1)
    {
        *pRetValue = 0;
        return !mbchar ? EINVAL : ERANGE;
    }
    mbchar[0] = static_cast<char>(wchar);
    if (sizeInBytes > 1) mbchar[1] = '\0';
    *pRetValue = 1;
    return 0;
}
