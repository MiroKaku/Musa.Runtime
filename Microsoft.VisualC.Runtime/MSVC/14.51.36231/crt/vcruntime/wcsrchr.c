/***
*wcsrchr.c - find last occurrence of wchar_t character in wide string
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines wcsrchr() - find the last occurrence of a given character
*       in a string (wide-characters).
*
*******************************************************************************/

#include <vcruntime_internal.h>

/***
*wchar_t *wcsrchr(string, ch) - find last occurrence of ch in wide string
*
*Purpose:
*       Finds the last occurrence of ch in string.  The terminating
*       null character is used as part of the search (wide-characters).
*
*Entry:
*       wchar_t *string - string to search in
*       wchar_t ch - character to search for
*
*Exit:
*       returns a pointer to the last occurrence of ch in the given
*       string
*       returns NULL if ch does not occur in the string
*
*Exceptions:
*
*******************************************************************************/

static wchar_t * __cdecl wcsrchr_simple (
        const wchar_t * string,
        wchar_t ch
        )
{
        wchar_t *start = (wchar_t *)string;

        while (*string++)                       /* find end of string */
                ;
                                                /* search towards front */
        while (--string != start && *string != ch)
                ;

        if (*string == ch)             /* wchar_t found ? */
                return( (wchar_t *)string );

        return(NULL);
}

#if defined(_M_ARM64) || defined(_M_HYBRID_X86_ARM64) || defined(_M_ARM64EC)

// ARM64 Neon Intrinsics variant
// For long strings, this is faster than the naive version.
// But for short strings there is overhead.

#include <arm64string.h>

// Traverse the string forwards, only once.
// Collect possible matches along the way.

wchar_t * __cdecl wcsrchr (
        const wchar_t * string,
        wchar_t ch
        )
{
    vector_t *src_a, characters, match;
    vector_t chmatch, zeromatch, orrmatches;
    __n64 uaddlvq32, maskv;
    unsigned __int64 chmask, zeromask, mask;
    unsigned long offset, ch_bitoffset, zero_bitoffset;
    wchar_t *found = (wchar_t)0;

    // If the string is not 2-byte-aligned, which should be
    // rare, fall back to a wchar-by-wchar loop.

    if ((intptr_t)(string) & 0x1) {
        return wcsrchr_simple(string, ch);
    }

    if (ch == 0) {
        if (*string == 0) {
            return (wchar_t *)string;
        } else {
            return wcschr_zero_internal(string);
        }
    }

    // Start by getting the aligned XMMWORD containing the first
    // characters of the string. This is done first to partially
    // cover any memory access latency.
    // Use 16 byte alignment throughout, to guarantee page-safe loads.

    src_a = (vector_t*)N128_ALIGN(string);

    // Now create patterns to check for a terminating zero or match.
    // These characters are copied to every position of a XMMWORD.

    match = neon_dupqr16(ch);

    // prepare to mask off any bits before the beginning of the string.

    offset = N128_OFFSET(string);

    {
        // Check initial full or partial XMMWORD

        characters = *src_a;

        // Compare against each pattern to get flags for each match

        chmatch = neon_cmeqq16(characters, match);
        zeromatch = neon_cmeqzq16(characters);

        maskv = neon_shrn_16(chmatch, 4);
        chmask = neon_umov64(maskv, 0);

        maskv = neon_shrn_16(zeromatch, 4);
        zeromask = neon_umov64(maskv, 0);

        // For the initial XMMWORD mask off any bits before the beginning
        // of the string.

        chmask = (chmask >> (offset << 2));
        zeromask = (zeromask >> (offset << 2));

        if ((chmask != 0) || (zeromask != 0)) {

            if (zeromask == 0) {

                // There is no zero match in this vector.
                // Record the offset of the LAST character match,
                // and advance to the next vector.

                _BitScanReverse64(&ch_bitoffset, chmask);
                ch_bitoffset = (ch_bitoffset >> 3);

                found = (wchar_t*)((ch_bitoffset << 1) + (intptr_t)(string));
            } else {

                if (chmask == 0) {

                    // The next match is the end of the string.

                    return found;
                }

                // Search the FIRST zero match

                _BitScanForward64(&zero_bitoffset, zeromask);

                // We have zero match after 1 or more character matches in this vector.
                // Mask off all character matches after the FIRST zero match,
                // RETURN the bit position of the LAST character match

                // Found zero match in first block. 1 <= zero_bitoffset <= 63
                chmask = chmask & ((1ull << zero_bitoffset) - 1);
                if (_BitScanReverse64(&ch_bitoffset, chmask)) {
                    ch_bitoffset = (ch_bitoffset >> 3);
                    found = (wchar_t*)((ch_bitoffset << 1) + (intptr_t)(string));
                }

                return found;
            }
        }
    }

    for (;;) {

        // Check each XMMWORD until the end of the string is found.

        characters = *(++src_a);

        // Compare against each pattern to get flags for each match

        chmatch = neon_cmeqq16(characters, match);
        zeromatch = neon_cmeqzq16(characters);

        orrmatches = neon_orrq(chmatch, zeromatch);
        uaddlvq32 = neon_uaddlvq32(orrmatches);
        mask = neon_umov64(uaddlvq32, 0);

        if (mask != 0) {
            maskv = neon_shrn_16(chmatch, 4);
            chmask = neon_umov64(maskv, 0);

            maskv = neon_shrn_16(zeromatch, 4);
            zeromask = neon_umov64(maskv, 0);

            if (zeromask == 0) {

                // There is no zero match in this vector.
                // Record the offset of the LAST character match,
                // and advance to the next vector.

                _BitScanReverse64(&ch_bitoffset, chmask);
                ch_bitoffset = (ch_bitoffset >> 3);

                found = (wchar_t*)((ch_bitoffset << 1) + (intptr_t)(src_a));

            } else {

                if (chmask == 0) {

                    // The next match is the end of the string.

                    return found;
                }

                // Search the FIRST zero match

                _BitScanForward64(&zero_bitoffset, zeromask);

                // We have zero match after 1 or more character matches in this vector.
                // Mask off all character matches after the FIRST zero match,
                // RETURN the bit position of the LAST character match

                chmask = chmask & ((1ull << zero_bitoffset) - 1);
                if (_BitScanReverse64(&ch_bitoffset, chmask)) {
                    ch_bitoffset = (ch_bitoffset >> 3);
                    found = (wchar_t*)((ch_bitoffset << 1) + (intptr_t)(src_a));
                }

                return found;
            }
        }
    }
}

#else

wchar_t * __cdecl wcsrchr (
        const wchar_t * string,
        wchar_t ch
        )
{
    return wcsrchr_simple(string, ch);
}

#endif
