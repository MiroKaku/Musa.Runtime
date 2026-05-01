;
; strlen.asm
;
;      Copyright (c) Microsoft Corporation.  All rights reserved.
;
; Optimized strlen and strnlen implementations for ARM64.
;
#include "ksarm64.h"

        ; size_t strlen(const char *str);                                // AV's when str == NULL
        ; size_t strnlen(const char *str, size_t numberOfElements);      // AV's when str == NULL

        ; This file could also define strnlen_s. strnlen_s is currently defined in the header string.h in C
        ; using a check for null and a call to strnlen. This avoids making the call in the case where the string is null,
        ; which should be infrequent. However it makes code larger by inlining that check everywhere strnlen_s is called.
        ; A better alternative would be to modify the standard headers and define strnlen_s here. It would be just one
        ; instruction because the required return value in x0 is already 0 when null is passed for the first parameter.
        ;
        ; EXPORT strnlen_s [FUNC]
        ; LEAF_ENTRY strnlen_s
        ; cbz     x0, AnyRet         ; add the label AnyRet in front of any ret instruction you want
        ;                            ; fallthrough into strnlen code
        ; ALTERNATE_ENTRY strnlen    ; change LEAF_ENTRY for strnlen to ALTERNATE_ENTRY
        ; ...                        ; current body of strnlen
        ; LEAF_END                   ; change strnlen leaf_end to strnlen_s.

        AREA |.text$mn$21|, CODE, READONLY, ALIGN=5

        ; With strlen we will usually read some bytes past the end of the string. To avoid getting an AV
        ; when a byte-by-byte implementation would not, we must ensure that we never cross a page boundary with a
        ; vector load, so we must align the vector loads to 16-byte-aligned boundaries.
        ;
        ; For strnlen we know the buffer length and so we won't read any bytes beyond the end of the buffer. This means
        ; we have a choice whether to arrange our vector loads to be 16-byte aligned. (Note that on arm64 a vector load
        ; only produces an alignment fault when the vector *elements* are misaligned, so a "16B" vector load will never
        ; give an alignment fault for user memory). Aligning the vector loads on 16-byte boundaries saves one cycle
        ; per vector load instruction. The cost of forcing 16-byte aligned loads is the 10 instructions preceding the
        ; 'NoNeedToAlign' label below. On Cortex-A57, the execution latency of those 10 instructions is 27 cycles,
        ; assuming no branch mispredict on the 'beq'. To account for the cost of an occasional mispredict we guess a
        ; mispredict rate of 2% and a mispredict cost of 50 cycles, or 1 cycle per call amortized, 28 total. 28 * 16 = 448.
        ; In this analysis we are ignoring the chance of extra cache misses due to loads crossing cache lines when
        ; they are not 16-byte aligned. When the vector loads span cache line boundaries each cache line is referenced
        ; one more time than it is when the loads are aligned. But we assume that the cache line stays loaded for the
        ; short time we need to do all the references to it, and so one extra reference won't matter.
        ; It is expected that the number of cycles (28) will stay about the same for future processor models. If it
        ; changes radically, it will be worth converting the EQU to a global, using ldr to load it instead of a
        ; mov-immediate, and dynamically setting the global during CRT startup based on processor model.

__strnlen_forceAlignThreshold  EQU 448                     ; code below assumes must be >= 32

        ; If a strlen is performed on an unterminated buffer, strlen may try to access an invalid address
        ; and generate an access violation. The prime imperative is for us not to generate an AV by loading
        ; characters beyond the end of a valid string. But also, in the case of an invalid string that should
        ; generate an AV, we need to have the AV report the proper bad address. If we only perform 16-byte aligned
        ; vector loads then the first time we touch an invalid page we will be loading from offset 0 in that
        ; page, which is the correct address for the AV to report. Even though we will usually load some bytes
        ; from beyond the end of the string, we won't load bytes beyond the end of the page unless the string
        ; extends into the next page. This is the primary purpose for forcing the vector loads to be
        ; 16-byte aligned.

        ; There is a slight performance boost for using aligned vector loads vs. unaligned ones but
        ; it is only worth the cost of aligning for longer strings (at least 512 chars).

        ; For strings less than 16 characters long the byte-by-byte loop will be about as fast as the
        ; compiler could produce, so we're not losing any performance vs. compiled C in any case.

#if !defined(_UPLEVEL_BUILD_)
        ARM64EC_ENTRY_THUNK A64NAME(strlen),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(strlen),|.text$$mn$$21|,5
#else
        LEAF_ENTRY_COMDAT A64NAME(_strlen_unused_),|.text$$mn$$21|,5
#endif

        ; check for empty string to avoid huge perf degradation in this case.
        ldrb    w2, [x0], #0
        cbz     w2, EmptyStr

        mov     x5, x0                                      ; keep original x0 value for the final 'sub'
        ; calculate number of bytes until first 16-byte alignment point

        ands    x1, x5, #15                                 ; x1 = (addr mod 16)
        beq     StrlenMainLoop                              ; no need to force alignment if already aligned

        ; we need to align, check whether we are within 16 bytes of the end of the page

        ands    x2, x5, #4095
        cmp     x2, #4080
        bgt     AlignByteByByte                             ; too close to end of page, must align byte-by-byte

        ; safe to do one unaligned 16-byte vector load to force alignment

        ld1     v0.16b, [x5]                                ; don't post-increment x5
        uminv   b1, v0.16b
        fmov    w2, s1                                      ; fmov is sometimes 1 cycle faster than 'umov w2, v1.b[0]'
        cbz     w2, FindNullInVector                        ; jump when string <= 15 bytes long & not near end of page
        add     x5, x5, #16                                 ; move x5 forward only to aligned address
        and     x5, x5, 0xFFFFFFFFFFFFFFF0                  ; first iter of StrlenMainLoop will retest some bytes we already tested

StrlenMainLoop                                              ; test 16 bytes at a time until we find it
        ld1     v0.16b, [x5], #16
        uminv   b1, v0.16b                                  ; use unsigned min to look for a zero byte; too bad it doesn't set CC
        fmov    w2, s1                                      ; need to move min byte into gpr to test it
        cbnz    w2, StrlenMainLoop                          ; fall through when any one of the bytes in v0 is zero

        sub     x5, x5, #16                                 ; undo the last #16 post-increment of x5

FindNullInVector                                            ; this label is also the target of a jump from strnlen
        cmeq    v0.16b, v0.16b, #0                          ; compute a 128-bit mask of 0xFF or 0x00
        shrn    v0.8b, v0.8h, #4                            ; narrow the 128-bit mask to a 64-bit mask of 0xF or 0x0
        fmov    x2, d0                                      ;
        rbit    x2, x2                                      ; reverse the bit order for clz
        clz     x2, x2                                      ; count the leading number of zeroes until the first bit set
        add     x5, x5, x2, LSR #2                          ; which is the offset we need to add to x5 to point at the null byte
        sub     x0, x5, x0                                  ; subtract ptr to null char from ptr to first char to get the strlen
        ret

ByteByByteFoundIt                                           ; this label is also the target of a jump from strnlen
        sub     x5, x5, #1                                  ; Undo the final post-increment that happened on the load of the null char.
        sub     x0, x5, x0                                  ; With x5 pointing at the null char, x5-x0 is the strlen
        ret

AlignByteByByte
        sub     x1, x1, #16                                 ; x1 = (addr mod 16) - 16
        neg     x1, x1                                      ; x1 = 16 - (addr mod 16) = count for byte-by-byte loop
ByteByByteLoop                                              ; test one byte at a time until we are 16-byte aligned
        ldrb    w2, [x5], #1
        cbz     w2, ByteByByteFoundIt                       ; branch if byte-at-a-time testing finds the null
        subs    x1, x1, #1
        bgt     ByteByByteLoop                              ; fall through when not found and 16-byte aligned
        b       StrlenMainLoop

EmptyStr
        mov     x0, 0
        ret

        LEAF_END

        END
