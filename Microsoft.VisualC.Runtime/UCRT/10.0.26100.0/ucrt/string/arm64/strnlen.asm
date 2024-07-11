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

#define srcin          x0
#define cntin          x1
#define result         x0
#define src            x2
#define synd           x3
#define shift          x4
#define tmp            x4
#define cntrem         x5

#define qdata          q0
#define vdata          v0
#define vhas_chr       v1
#define vend           v2
#define dend           d2

/*
   Core algorithm:
   Process the string in 16-byte aligned chunks. Compute a 64-bit mask with
   four bits per byte using the shrn instruction. A count trailing zeros then
   identifies the first zero byte.  */

        ARM64EC_ENTRY_THUNK A64NAME(strnlen),1,0,|.text$$mn$$21|
        LEAF_ENTRY A64NAME(strnlen),|.text$$mn$$21|,5

        bic        src, srcin, 15
        cbz        cntin, nomatch
        ld1        {vdata.16b}, [src]
        cmeq       vhas_chr.16b, vdata.16b, 0
        lsl        shift, srcin, 2
        shrn       vend.8b, vhas_chr.8h, 4                /* 128->64 */
        fmov       synd, dend
        lsr        synd, synd, shift
        cbz        synd, start_loop
finish
        rbit       synd, synd
        clz        synd, synd
        lsr        result, synd, 2
        cmp        cntin, result
        csel       result, cntin, result, ls
        ret

nomatch
        mov        result, cntin
        ret

start_loop
        sub        tmp, src, srcin
        add        tmp, tmp, 17
        subs       cntrem, cntin, tmp
        b.lo       nomatch

        /* Make sure that it won't overread by a 16-byte chunk */
        tbz        cntrem, 4, loop32_2
        sub        src, src, 16
        b          loop32

        ALIGN 32
loop32
        ldr        qdata, [src, 32]!
        cmeq       vhas_chr.16b, vdata.16b, 0
        umaxp      vend.16b, vhas_chr.16b, vhas_chr.16b                /* 128->64 */
        fmov       synd, dend
        cbnz       synd, end
loop32_2
        ldr        qdata, [src, 16]
        subs       cntrem, cntrem, 32
        cmeq       vhas_chr.16b, vdata.16b, 0
        b.lo       end_2
        umaxp      vend.16b, vhas_chr.16b, vhas_chr.16b                /* 128->64 */
        fmov       synd, dend
        cbz        synd, loop32
end_2
        add        src, src, 16
end
        shrn       vend.8b, vhas_chr.8h, 4                             /* 128->64 */
        sub        result, src, srcin
        fmov       synd, dend
        rbit       synd, synd
        clz        synd, synd
        add        result, result, synd, lsr 2
        cmp        cntin, result
        csel       result, cntin, result, ls
        ret

        LEAF_END

        END
