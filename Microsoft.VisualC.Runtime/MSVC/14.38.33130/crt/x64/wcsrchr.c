/*
 *  Copyright (C) 2009-2010 Intel Corporation.
 *
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/***
*wcsrchr.c - find last occurrence of wchar_t character in wide string
*
*Purpose:
*       defines wcsrchr() - find the last occurrence of a given character
*       in a string (wide-characters).
*
*******************************************************************************/

#include <vcruntime_internal.h>
#include <intrin.h>

// These flags select the operation performed by _mm_cmp?str? functions.

// PCMPxSTRx character type
#define f_pcmp_ub  0x00
#define f_pcmp_uw  0x01
#define f_pcmp_sb  0x02
#define f_pcmp_sw  0x03
// PCMPxSTRx aggregation operation
#define f_pcmp_ea  0x00
#define f_pcmp_rng 0x04
#define f_pcmp_ee  0x08
#define f_pcmp_eo  0x0C
// PCMPxSTRx result polarity
#define f_pcmp_pp  0x00
#define f_pcmp_np  0x10
#define f_pcmp_mn  0x30
// PCMPxSTRI index order
#define f_pcmp_ls  0x00
#define f_pcmp_ms  0x40
// PCMPxSTRM result unit size
#define f_pcmp_bit  0x00
#define f_pcmp_byte 0x40

// Define flag combinations to use.
#define f_srch_rng (f_pcmp_uw | f_pcmp_rng | f_pcmp_np | f_pcmp_ls)
#define f_srch_set (f_pcmp_uw | f_pcmp_ea | f_pcmp_pp | f_pcmp_ms)

#define XMM_SIZE sizeof(__m128i)
#define XMM_OFFSET(p) ((XMM_SIZE - 1) & (intptr_t)(p))
#define XMM_CHARS (XMM_SIZE / sizeof(wchar_t))
#define XMM_CHAR_ALIGNED(p) (0 == (((intptr_t)(p) + sizeof(wchar_t) - 1) & (0-sizeof(wchar_t)) & (XMM_SIZE-1)))

#define PAGE_SIZE ((intptr_t)0x1000)
#define PAGE_OFFSET(p) ((PAGE_SIZE-1) & (intptr_t)(p))
#define XMM_PAGE_SAFE(p) (PAGE_OFFSET(p) <= (PAGE_SIZE - XMM_SIZE))

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

_Check_return_
wchar_t * __cdecl wcsrchr (
    _In_z_  const wchar_t * string,
    _In_    wchar_t ch
        )
{
    if (__isa_available < __ISA_AVAILABLE_SSE42)
    {
        const wchar_t *start = string;

        while (*string++)                       /* find end of string */
            ;
                                                /* search towards front */
        while (--string != start && *string != (wchar_t)ch)
            ;

        if (*string == (wchar_t)ch)             /* wchar_t found ? */
            return (wchar_t *)string;

        return(NULL);
    }
    else
    {
        __m128i pattern, characters;
        const wchar_t *last_match = NULL;

        // Step through string until it is aligned for XMMWORD access.
        // This happens when the last byte of the next character is
        // in the first character of the next aligned XMMWORD. In this
        // case loading the next character or XMMWORD will always access
        // the same memory pages.
        for (; !XMM_CHAR_ALIGNED(string); ++string)
        {
            if (*string == ch) last_match = string;
            if (*string == 0) return (wchar_t *)last_match;
        }

        // Search string by XMMWORD blocks using STTNI. Because you can't
        // explicitly search for a zero character we implement that as a
        // special case where we search for the end of a string of non-zero
        // characters using the search-range function.
        if (0 == ch)
        {
            // Pattern is set of all valid character values except zero.
            pattern = _mm_cvtsi32_si128(0xffff0001);
            for (;;)
            {
                characters = _mm_loadu_si128((__m128i*)string);
                if (_mm_cmpistrz(pattern, characters, f_srch_rng))
                {
                    return (wchar_t *)(_mm_cmpistri(pattern, characters, f_srch_rng) + string);
                }
                string += XMM_CHARS;
            }
        }
        else
        {
            // Pattern is one-character string of the target character.
            pattern = _mm_cvtsi32_si128(ch);
            for (;;)
            {
                characters = _mm_loadu_si128((__m128i*)string);
                if (_mm_cmpistrc(pattern, characters, f_srch_set))
                {
                    last_match = _mm_cmpistri(pattern, characters, f_srch_set) + string;
                }
                if (_mm_cmpistrz(pattern, characters, f_srch_set))
                {
                    return (wchar_t *)last_match;
                }
                string += XMM_CHARS;
            }
        }
    }
}
