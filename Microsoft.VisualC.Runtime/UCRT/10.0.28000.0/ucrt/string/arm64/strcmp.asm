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

#if !defined(_M_ARM64EC)

        EXPORT A64NAME(strcmp)    [FUNC]

#endif

        AREA |.text$mn$21|, CODE, READONLY, ALIGN=5

#define REP8_01    0x0101010101010101
#define REP8_7f    0x7f7f7f7f7f7f7f7f

#define src1       x0
#define src2       x1
#define result     x0

#define data1      x2
#define data1w     w2
#define data2      x3
#define data2w     w3
#define has_nul    x4
#define diff       x5
#define off1       x5
#define syndrome   x6
#define tmp        x6
#define data3      x7
#define zeroones   x8
#define shift      x9
#define off2       x10

; NUL detection works on the principle that (X - 1) & (~X) & 0x80
; (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
; can be done in parallel across the entire word.

        ARM64EC_ENTRY_THUNK A64NAME(strcmp),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(strcmp),|.text$$mn$$21|,5

        sub    off2, src2, src1
        mov    zeroones, REP8_01
        and    tmp, src1, 7
        tst    off2, 7
        bne    misaligned8
        cbnz   tmp, mutual_align

loop_aligned
        ldr    data2, [src1, off2]
        ldr    data1, [src1], 8

start_realigned
        sub    has_nul, data1, zeroones
        orr    tmp, data1, REP8_7f
        bics   has_nul, has_nul, tmp    ; Non-zero if NUL terminator.
        ccmp   data1, data2, 0, eq
        beq    loop_aligned
        eor    diff, data1, data2
        orr    syndrome, diff, has_nul

end
        rev    syndrome, syndrome
        rev    data1, data1
        rev    data2, data2
        clz    shift, syndrome
        ; The most-significant-non-zero bit of the syndrome marks either the
        ; first bit that is different, or the top bit of the first zero byte.
        ; Shifting left now will bring the critical information into the
        ; top bits.
        lsl    data1, data1, shift
        lsl    data2, data2, shift
        ; But we need to zero-extend (char is unsigned) the value and then
        ; perform a signed 32-bit subtraction.
        lsr    data1, data1, 56
        cmp    data1, data2, lsr 56

        ; Although the C/C++ standard does not regulate the return value if
        ; it is non-zero, MS strcmp implementation has been returning 1, 0,
        ; -1 historically. To maximize compat, we normalize the result before
        ; returning.
normalize_result
        csetm  tmp, lt
        csinc  result, tmp, xzr, le
        ret

mutual_align
        ; Sources are mutually aligned, but are not currently at an
        ; alignment boundary.  Round down the addresses and then mask off
        ; the bytes that precede the start point.
        bic    src1, src1, 7
        ldr    data2, [src1, off2]
        ldr    data1, [src1], 8
        neg    shift, src2, lsl 3    ; Bits to alignment -64.
        mov    tmp, -1
        lsr    tmp, tmp, shift
        orr    data1, data1, tmp
        orr    data2, data2, tmp
        b      start_realigned

misaligned8
        ; Align SRC1 to 8 bytes and then compare 8 bytes at a time, always
        ; checking to make sure that we don't access beyond the end of SRC2.
        cbz    tmp, src1_aligned

do_misaligned
        ldrb   data1w, [src1], 1
        ldrb   data2w, [src2], 1
        cmp    data1w, 0
        ccmp   data1w, data2w, 0, ne    ; NZCV = 0b0000.
        bne    done
        tst    src1, 7
        bne    do_misaligned

src1_aligned
        neg    shift, src2, lsl 3
        bic    src2, src2, 7
        ldr    data3, [src2], 8
        lsr    tmp, zeroones, shift
        orr    data3, data3, tmp
        sub    has_nul, data3, zeroones
        orr    tmp, data3, REP8_7f
        bics   has_nul, has_nul, tmp
        bne    tail

        sub    off1, src2, src1

loop_unaligned
        ldr    data3, [src1, off1]
        ldr    data2, [src1, off2]
        sub    has_nul, data3, zeroones
        orr    tmp, data3, REP8_7f
        ldr    data1, [src1], 8
        bics   has_nul, has_nul, tmp
        ccmp   data1, data2, 0, eq
        beq    loop_unaligned

        lsl    tmp, has_nul, shift
        eor    diff, data1, data2
        orr    syndrome, diff, tmp
        cbnz   syndrome, end

tail
        ldr    data1, [src1]
        neg    shift, shift
        lsr    data2, data3, shift
        lsr    has_nul, has_nul, shift
        eor    diff, data1, data2
        orr    syndrome, diff, has_nul
        b      end

done
        cmp    data1, data2
        b      normalize_result

        LEAF_END

        END