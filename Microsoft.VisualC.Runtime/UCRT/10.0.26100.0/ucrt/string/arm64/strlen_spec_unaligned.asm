/*
Copyright (c) 1999-2022, Arm Limited.
Licensed under the MIT license.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ksarm64.h"

        AREA |.text$mn$21|, CODE, READONLY, ALIGN=5

#define srcin       x0
#define len         x0

#define src         x1
#define data1       x2
#define data2       x3
#define has_nul1    x4
#define has_nul2    x5
#define tmp1        x4
#define tmp2        x5
#define tmp3        x6
#define tmp4        x7
#define zeroones    x8

#define maskv       v0
#define maskd       d0
#define dataq1      q1
#define dataq2      q2
#define datav1      v1
#define datav2      v2
#define tmp         x2
#define tmpw        w2
#define synd        x3
#define syndw       w3
#define shift       x4

/* For the first 32 bytes, NUL detection works on the principle that
   (X - 1) & (~X) & 0x80 (=> (X - 1) & ~(X | 0x7f)) is non-zero if a
   byte is zero, and can be done in parallel across the entire word.  */

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define MIN_PAGE_SIZE 4096


        /* Core algorithm:

        Since strings are short on average, we check the first 32 bytes of the
        string for a NUL character without aligning the string.  In order to use
        unaligned loads safely we must do a page cross check first.

        If there is a NUL byte we calculate the length from the 2 8-byte words
        using conditional select to reduce branch mispredictions (it is unlikely
        strlen will be repeatedly called on strings with the same length).

        If the string is longer than 32 bytes, align src so we don't need further
        page cross checks, and process 32 bytes per iteration using a fast SIMD
        loop.

        If the page cross check fails, we read 32 bytes from an aligned address,
        and ignore any characters before the string.  If it contains a NUL
        character, return the length, if not, continue in the main loop.  */

#if !defined(_UPLEVEL_BUILD_)
        ARM64EC_ENTRY_THUNK A64NAME(__strlen_spec_unaligned),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(__strlen_spec_unaligned),|.text$$mn$$21|,5
#else
        ARM64EC_ENTRY_THUNK A64NAME(strlen),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(strlen),|.text$$mn$$21|,5
#endif

        and        tmp1, srcin, MIN_PAGE_SIZE - 1
        cmp        tmp1, MIN_PAGE_SIZE - 32
        b.hi       page_cross

        /* Look for a NUL byte in the first 16 bytes.  */
        ldp        data1, data2, [srcin]
        mov        zeroones, REP8_01

        sub        tmp1, data1, zeroones
        orr        tmp2, data1, REP8_7f
        sub        tmp3, data2, zeroones
        orr        tmp4, data2, REP8_7f
        bics       has_nul1, tmp1, tmp2
        bic        has_nul2, tmp3, tmp4
        ccmp       has_nul2, 0, 0, eq
        b.eq       bytes16_31

        /* Find the exact offset of the first NUL byte in the first 16 bytes
           from the string start.  Enter with C = has_nul1 == 0.  */
        csel       has_nul1, has_nul1, has_nul2, cc
        mov        len, 8
        rev        has_nul1, has_nul1
        csel       len, xzr, len, cc
        clz        tmp1, has_nul1
        add        len, len, tmp1, lsr 3
        ret

        /* Look for a NUL byte at offset 16..31 in the string.  */
bytes16_31
        ldp        data1, data2, [srcin, 16]
        sub        tmp1, data1, zeroones
        orr        tmp2, data1, REP8_7f
        sub        tmp3, data2, zeroones
        orr        tmp4, data2, REP8_7f
        bics       has_nul1, tmp1, tmp2
        bic        has_nul2, tmp3, tmp4
        ccmp       has_nul2, 0, 0, eq
        b.eq       loop_entry

        /* Find the exact offset of the first NUL byte at offset 16..31 from
           the string start.  Enter with C = has_nul1 == 0.  */
        csel       has_nul1, has_nul1, has_nul2, cc
        mov        len, 24
        rev        has_nul1, has_nul1
        mov        tmp3, 16
        clz        tmp1, has_nul1
        csel       len, tmp3, len, cc
        add        len, len, tmp1, lsr 3
        ret

        nop
loop_entry
        bic        src, srcin, 31
        b          loop

        ALIGN 32
loop
        ldp        dataq1, dataq2, [src, 32]!
        uminp      maskv.16b, datav1.16b, datav2.16b
        uminp      maskv.16b, maskv.16b, maskv.16b
        cmeq       maskv.8b, maskv.8b, 0
        fmov       synd, maskd
        cbz        synd, loop

        /* Low 32 bits of synd are non-zero if a NUL was found in datav1.  */
        cmeq       maskv.16b, datav1.16b, 0
        sub        len, src, srcin
        cbnz       syndw, %F1
        cmeq       maskv.16b, datav2.16b, 0
        add        len, len, 16
1
        /* Generate a bitmask and compute correct byte offset.  */
        shrn       maskv.8b, maskv.8h, 4
        fmov       synd, maskd
        rbit       synd, synd
        clz        tmp, synd
        add        len, len, tmp, lsr 2
        ret

page_cross
        bic        src, srcin, 31
        mov        tmpw, 0x0c03
        movk       tmpw, 0xc030, lsl 16
        ld1        {datav1.16b, datav2.16b}, [src]
        dup        maskv.4s, tmpw
        cmeq       datav1.16b, datav1.16b, 0
        cmeq       datav2.16b, datav2.16b, 0
        and        datav1.16b, datav1.16b, maskv.16b
        and        datav2.16b, datav2.16b, maskv.16b
        addp       maskv.16b, datav1.16b, datav2.16b
        addp       maskv.16b, maskv.16b, maskv.16b
        fmov       synd, maskd
        lsl        shift, srcin, 1
        lsr        synd, synd, shift
        cbz        synd, loop

        rbit       synd, synd
        clz        len, synd
        lsr        len, len, 1
        ret

empty_str
        mov        len, 0
        ret

        LEAF_END

        END
