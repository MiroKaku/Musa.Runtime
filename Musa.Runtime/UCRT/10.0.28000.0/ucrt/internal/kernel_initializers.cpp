//
// kernel_initializers.cpp -- Kernel-mode stub initializers for UCRT
//
// Provides empty implementations for UCRT CRT initializers/terminators
// that have no meaning in kernel mode.
//
#include <corecrt_internal.h>

// ---- CRT Initializers ----


// command_line initializer — kernel mode: no command line
extern "C" bool __cdecl __acrt_initialize_command_line() { return true; }
extern "C" bool __cdecl __acrt_uninitialize_command_line(bool) { return true; }
// clock initializer -- from base SDK time/clock.cpp
extern "C" void __cdecl __acrt_clock_initializer() { }

// ---- Application Type / Multibyte Init ----

// _query_app_type -- from base SDK lowio/osfinfo.cpp (appmodel.h)
extern "C" _crt_app_type __cdecl _query_app_type()
{
    return _crt_unknown_app;
}

extern "C" bool __cdecl __acrt_initialize_multibyte()
{
    return true;
}
extern "C" void __cdecl __acrt_fmode_initializer() { }






// In kernel mode, after lowio init, force stdin/stdout/stderr to act like
// "no console" handles so _isatty(_fileno(stdout)) returns 0 (no TTY) and
// stdio init sets _iob[0..2]._file = _NO_CONSOLE_FILENO.
//
// __acrt_initialize_lowio always sets the FDEV flag for stdio handles when
// the kernel-mode standard handle is non-null (e.g., debug output device),
// which violates the test expectation that there is no terminal in the
// kernel-mode driver context.

#include <corecrt_internal_lowio.h>

extern "C" bool __cdecl __acrt_kernel_fixup_stdio_handles()
{
    if (__pioinfo[0] == nullptr)
    {
        return true;
    }

    for (int fh = 0; fh < 3; ++fh)
    {
        __crt_lowio_handle_data* const pio = _pioinfo(fh);
        pio->osfhnd = _NO_CONSOLE_FILENO;
        pio->osfile = static_cast<unsigned char>(pio->osfile & ~FDEV);
    }
    return true;
}

extern "C" bool __cdecl __acrt_kernel_uninitialize_stdio_handles(bool)
{
    return true;
}
