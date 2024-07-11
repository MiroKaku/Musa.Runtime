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

        EXPORT A64NAME(strncmp)    [FUNC]

#endif

        AREA |.text$mn$21|, CODE, READONLY, ALIGN=5

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f

#define src1        x0
#define src2        x1
#define limit       x2
#define result      x0

#define data1       x3
#define data1w      w3
#define data2       x4
#define data2w      w4
#define has_nul     x5
#define diff        x6
#define syndrome    x7
#define tmp1        x8
#define tmp2        x9
#define tmp3        x10
#define zeroones    x11
#define pos         x12
#define mask        x16
#define endloop     x17
#define count       mask
#define offset      pos
#define neg_offset  x15

; NUL detection works on the principle that (X - 1) & (~X) & 0x80
; (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
; can be done in parallel across the entire word.

        ARM64EC_ENTRY_THUNK A64NAME(strncmp),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(strncmp),|.text$$mn$$21|,5

        cbz     limit, ret0
        eor     tmp1, src1, src2
        mov     zeroones, #REP8_01
        tst     tmp1, #7
        and     count, src1, #7
        bne     misaligned8
        cbnz    count, mutual_align

loop_aligned
        ldr     data1, [src1], #8
        ldr     data2, [src2], #8
start_realigned
        subs    limit, limit, #8
        sub     tmp1, data1, zeroones
        orr     tmp2, data1, #REP8_7f
        eor     diff, data1, data2          ; Non-zero if differences found.
        csinv   endloop, diff, xzr, hi      ; Last Dword or differences.
        bics    has_nul, tmp1, tmp2         ; Non-zero if NUL terminator.
        ccmp    endloop, #0, #0, eq
        beq     loop_aligned
        ; End of main loop

full_check
        orr     syndrome, diff, has_nul
        add     limit, limit, 8              ; Rewind limit to before last subs.
syndrome_check
        ; Limit was reached. Check if the NUL byte or the difference
        ; is before the limit.
        rev     syndrome, syndrome
        rev     data1, data1
        clz     pos, syndrome
        rev     data2, data2
        lsl     data1, data1, pos
        cmp     limit, pos, lsr #3
        lsl     data2, data2, pos
        ; But we need to zero-extend (char is unsigned) the value and then
        ; perform a signed 32-bit subtraction.
        lsr     data1, data1, #56
        sub     result, data1, data2, lsr #56
        csel    result, result, xzr, hi
        ret

mutual_align
        ; Sources are mutually aligned, but are not currently at an
        ; alignment boundary.  Round down the addresses and then mask off
        ; the bytes that precede the start point.
        ; We also need to adjust the limit calculations, but without
        ; overflowing if the limit is near ULONG_MAX.
        bic     src1, src1, #7
        bic     src2, src2, #7
        ldr     data1, [src1], #8
        neg     tmp3, count, lsl #3          ; 64 - bits(bytes beyond align).
        ldr     data2, [src2], #8
        mov     tmp2, #~0
        lsr     tmp2, tmp2, tmp3             ; Shift (count & 63).

        ; Adjust the limit and ensure it doesn't overflow.
        adds    limit, limit, count
        csinv   limit, limit, xzr, lo
        orr     data1, data1, tmp2
        orr     data2, data2, tmp2
        b       start_realigned

        ; Don't bother with dwords for up to 16 bytes.
misaligned8
        cmp     limit, #16
        bhs     try_misaligned_words

byte_loop
        ; Perhaps we can do better than this.
        ldrb    data1w, [src1], #1
        ldrb    data2w, [src2], #1
        subs    limit, limit, #1
        ccmp    data1w, #1, #0, hi          ; NZCV = 0b0000.
        ccmp    data1w, data2w, #0, cs      ; NZCV = 0b0000.
        beq     byte_loop
done
        sub     result, data1, data2
        ret

        ; Align the SRC1 to a dword by doing a bytewise compare and then do
        ; the dword loop.
try_misaligned_words
        cbz     count, src1_aligned

        neg     count, count
        and     count, count, #7
        sub     limit, limit, count

page_end_loop
        ldrb    data1w, [src1], #1
        ldrb    data2w, [src2], #1
        cmp     data1w, #1
        ccmp    data1w, data2w, #0, cs      ; NZCV = 0b0000.
        bne     done
        subs    count, count, #1
        bhi     page_end_loop

        ; The following diagram explains the comparison of misaligned strings.
        ; The bytes are shown in natural order. For little-endian, it is
        ; reversed in the registers. The "x" bytes are before the string.
        ; The "|" separates data that is loaded at one time.
        ; src1     | a a a a a a a a | b b b c c c c c | . . .
        ; src2     | x x x x x a a a   a a a a a b b b | c c c c c . . .

        ; After shifting in each step, the data looks like this:
        ;          STEP_A        ;     STEP_B        ;     STEP_C
        ; data1    a a a a a a a a     b b b c c c c c     b b b c c c c c
        ; data2    a a a a a a a a     b b b 0 0 0 0 0     0 0 0 c c c c c

        ; The bytes with "0" are eliminated from the syndrome via mask.

        ; Align SRC2 down to 16 bytes. This way we can read 16 bytes at a
        ; time from SRC2. The comparison happens in 3 steps. After each step
        ; the loop can exit, or read from SRC1 or SRC2.
src1_aligned
        ; Calculate offset from 8 byte alignment to string start in bits. No
        ; need to mask offset since shifts are ignoring upper bits.
        lsl     offset, src2, #3
        bic     src2, src2, #0xf
        mov     mask, -1
        neg     neg_offset, offset
        ldr     data1, [src1], #8
        ldp     tmp1, tmp2, [src2], #16
        lsl     mask, mask, neg_offset
        and     neg_offset, neg_offset, #63  ; Need actual value for cmp later.
        ; Skip the first compare if data in tmp1 is irrelevant.
        tbnz    offset, 6, misaligned_mid_loop

loop_misaligned
        ; STEP_A: Compare full 8 bytes when there is enough data from SRC2.
        lsr     data2, tmp1, offset
        lsl     tmp1, tmp2, neg_offset
        subs    limit, limit, #8
        orr     data2, data2, tmp1           ; 8 bytes from SRC2 combined from two regs.
        sub     has_nul, data1, zeroones
        eor     diff, data1, data2           ; Non-zero if differences found.
        orr     tmp3, data1, #REP8_7f
        csinv   endloop, diff, xzr, hi       ; If limit, set to all ones.
        bic     has_nul, has_nul, tmp3       ; Non-zero if NUL byte found in SRC1.
        orr     tmp3, endloop, has_nul
        cbnz    tmp3, full_check

        ldr     data1, [src1], #8
misaligned_mid_loop
        ; STEP_B: Compare first part of data1 to second part of tmp2.
        lsr     data2, tmp2, offset
        sub     has_nul, data1, zeroones
        orr     tmp3, data1, #REP8_7f
        eor     diff, data2, data1           ; Non-zero if differences found.
        bic     has_nul, has_nul, tmp3       ; Non-zero if NUL terminator.
        cmp     limit, neg_offset, lsr #3
        orr     syndrome, diff, has_nul
        bic     syndrome, syndrome, mask     ; Ignore later bytes.
        csinv   tmp3, syndrome, xzr, hi      ; If limit, set to all ones.
        cbnz    tmp3, syndrome_check

        ; STEP_C: Compare second part of data1 to first part of tmp1.
        ldp     tmp1, tmp2, [src2], #16
        cmp     limit, #8
        lsl     data2, tmp1, neg_offset
        eor     diff, data2, data1           ; Non-zero if differences found.
        orr     syndrome, diff, has_nul
        and     syndrome, syndrome, mask     ; Ignore earlier bytes.
        csinv   tmp3, syndrome, xzr, hi      ; If limit, set to all ones.
        cbnz    tmp3, syndrome_check

        ldr     data1, [src1], #8
        sub     limit, limit, #8
        b       loop_misaligned

ret0
        mov     result, #0
        ret

        LEAF_END

        END