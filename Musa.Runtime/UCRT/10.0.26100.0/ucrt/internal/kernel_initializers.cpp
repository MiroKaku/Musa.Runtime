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





