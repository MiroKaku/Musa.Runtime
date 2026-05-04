//
// kernel_initializers.cpp -- Kernel-mode stub initializers for UCRT
//
// Provides empty implementations for UCRT CRT initializers/terminators
// that have no meaning in kernel mode.
//
#include <corecrt_internal.h>

// ---- CRT Initializers ----

// stdio initializer/terminator -- requested via _CRT_LINKER_FORCE_INCLUDE in base SDK _file.cpp
extern "C" void __cdecl __acrt_stdio_initializer() { }
extern "C" void __cdecl __acrt_stdio_terminator() { }

// clock initializer -- from base SDK time/clock.cpp
extern "C" void __cdecl __acrt_clock_initializer() { }

// timeset initializer -- from base SDK time/timeset.cpp
extern "C" void __cdecl __acrt_timeset_initializer() { }

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
