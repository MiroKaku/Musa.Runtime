//
// isatty.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Defines _isatty(), which tests whether a file refers to a character device.
//
#include <corecrt_internal_lowio.h>

// VSO 41542369 - Ruby tries to locate the __pioinfo pointer by looking 
// backwards from the ret instruction. This is not supported in general,
// but in addition Ruby expects an [add rsp] instruction before the ret.
// Whether that pattern exists depends on what optimizations are applied.
// This creates AppCompat issues as Ruby based applications abort at start.
// We work around it having _isatty being a jmp to the real fucntion,
// followed by non-reachable code that Ruby looks for.
// See also isatty_ruby.asm

// Tests if the given file refers to a character device (e.g. terminal, console,
// printer, serial port, etc.).  Returns nonzero if so; zero if not.

#if defined(_ISATTY_ASM) && !defined(_ARM64EC_)
extern "C" int __cdecl _isatty_proc(int const fh)
#else
extern "C" int __cdecl _isatty(int const fh)
#endif
{
    _CHECK_FH_RETURN(fh, EBADF, 0);
    _VALIDATE_RETURN((fh >= 0 && (unsigned)fh < (unsigned)_nhandle), EBADF, 0);

    return static_cast<int>(_osfile(fh) & FDEV);
}
