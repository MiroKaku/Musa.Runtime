;
; wcslen.asm
;
;      Copyright (c) Microsoft Corporation.  All rights reserved.
;
; Optimized wcslen implementation for ARM64.
;
#include "ksarm64.h"

        ; size_t wcslen(const wchar_t *str);

        ; Note: this code assumes that the input parameter is always aligned to an even byte boundary.

        AREA |.text$mn$21|, CODE, READONLY, ALIGN=6

        ; With wcslen we will usually read some chars past the end of the string. To avoid getting an AV
        ; when a char-by-char implementation would not, we have to ensure that we never cross a page boundary with a
        ; vector load, so we must align the vector loads to 16-byte-aligned boundaries.

__wcsnlen_forceAlignThreshold  EQU 216                      ; code logic below assumes >= 16

        ARM64EC_ENTRY_THUNK A64NAME(wcslen),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(wcslen),|.text$$mn$$21|,6

        ; check for empty string to avoid huge perf degradation in this case.
        ldrh    w2, [x0], #0
        cbz     w2, EmptyStr

        mov     x5, x0                                      ; keep original x0 value for the final 'sub'
        tbnz    x0, #0, WCharAtATime                        ; check for misaligned characters. Must go char-by-char when
                                                            ; misaligned so that if there's an access violation it gets
                                                            ; generated on the correct address (one byte into a new page
                                                            ; instead of up to 15 bytes in if we had loaded a vector
                                                            ; from the last byte of the previous page)

        ; calculate number of bytes until first 16-byte alignment point

        ands    x1, x5, #15                                 ; x1 = (addr mod 16)
        beq     WcslenMainLoop                              ; no need to force alignment if already aligned

        ; we need to align, check whether we are within 16 bytes of the end of the page.
        ; branch if ((address mod PAGESIZE - 1) > (PAGESIZE - 16))

        and     x2, x5, #4095                               ; x2 = address mod (PAGESIZE - 1)
        cmp     x2, #4080                                   ; compare x2 to (PAGESIZE - 16)
        bgt     AlignSlowly                                 ; too close to end of page, must align one wchar at a time

        ; AlignFast: safe to do one 2-byte aligned vector load to force alignment to a 16-byte boundary

        ld1     v0.8h, [x5]                                 ; don't post-increment x5
        uminv   h1, v0.8h
        fmov    w2, s1                                      ; fmov is sometimes 1 cycle faster than "umov w2, v1.h[0]"
        cbz     w2, FindWideNullInVector                    ; jump when string < 15 bytes (<= 7 wchar_t's) long & not near end of page
        add     x5, x5, #16                                 ; move x5 forward only to aligned address.  (Assumes even address)
        and     x5, x5, 0xFFFFFFFFFFFFFFF0                  ; first iter of StrlenMainLoop will retest some bytes we already tested

        ; The code at WcslenMainLoop should be 64-byte aligned for best performance.
        ; Due to VSO#1106651, automatic padding with NOPs when in code areas is
        ; broken. The workaround is to use -721215457.
        ; MSFT:21876224 tracks removal of this workaround.
        ALIGN 64,0,-721215457,4

WcslenMainLoop                                              ; test 8 wchar_t's at a time until we find it
        ld1     v0.8h, [x5], #16
        uminv   h1, v0.8h                                   ; use unsigned min to look for a zero wchar_t; too bad it doesn't set CC
        fmov    w2, s1                                      ; need to move min wchar_t into gpr to test it
        cbnz    w2, WcslenMainLoop                          ; fall through when any one of the wchar_ts in v0 is zero

        sub     x5, x5, #16                                 ; undo the last #16 post-increment of x5

FindWideNullInVector
        cmeq    v0.8h, v0.8h, #0                            ; compute a 128-bit mask of 0xFFFF or 0x0000
        shrn    v0.8b, v0.8h, #4                            ; narrow the 128-bit mask to a 64-bit mask of 0xFF or 0x00
        fmov    x2, d0                                      ;
        rbit    x2, x2                                      ; reverse the bit order for clz
        clz     x2, x2                                      ; count the leading number of zeroes until the first bit set
        lsr     x2, x2, #3                                  ; right-shift x2 by 3 to calculate the byte position for zero wchar_t

        sub     x0, x5, x0                                  ; subtract ptr to null char from ptr to first char to get the string length in bytes
        add     x0, x2, x0, ASR #1                          ; divide x0 by 2 to get the number of wide chars and then add in the final vector char pos
        ret

AlignSlowly
        sub     x1, x1, #16                                 ; x1 = (addr mod 16) - 16
        sub     x1, xzr, x1, ASR #1                         ; x1 = -(((addr mod 16) - 16) / 2) = (16 - (addr mod 16)) / 2 = num wchar_ts

AlignLoop                                                   ; test one wchar_t at a time until we are 16-byte aligned
        ldrh    w2, [x5], #2
        cbz     w2, OneByOneFoundIt                         ; branch if found the null
        subs    x1, x1, #1
        bgt     AlignLoop                                   ; fall through when not found and reached 16-byte alignment
        b       WcslenMainLoop

WCharAtATime
        ldrh    w2, [x5], #2
        cbnz    w2, WCharAtATime                            ; when found use same exit sequence as when found during slow alignment

OneByOneFoundIt
        sub     x5, x5, #2                                  ; Undo the final post-increment that happened on the load of the null wchar_t.
        sub     x0, x5, x0                                  ; With x5 pointing at the null char, x5-x0 is the length in bytes
        asr     x0, x0, #1                                  ; divide by 2 to get length in wchar_ts
        ret

EmptyStr
        mov     x0, 0
        ret

        LEAF_END

        END
