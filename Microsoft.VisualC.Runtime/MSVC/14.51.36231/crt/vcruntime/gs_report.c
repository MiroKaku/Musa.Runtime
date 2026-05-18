//
// gs_report.c
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of functions used to report a security check failure.  This file
// corresponds to gs_report.c in the Windows sources.
//
#include <vcruntime_internal.h>



// __report_gsfailure() - Report security error.
//
// A /GS security error has been detected.  We terminate the program with __fastfail.
//
// NOTE: __report_gsfailure should not be called directly.  Instead, it should
// be entered due to a failure detected by __security_check_cookie.   That's
// because __security_check_cookie may perform more checks than just a mismatch
// against the global security cookie.
//
// ULONGLONG stack_cookie - the local cookie.  On x86, the local cookie is not
// passed in and is instead available in ECX, but since __report_gsfailure isn't
// __fastcall, it isn't a true argument.
#if defined _M_IX86
    #define GSFAILURE_PARAMETER
#elif defined _M_X64
    #define GSFAILURE_PARAMETER _In_ ULONGLONG stack_cookie
#elif defined _M_ARM64
    #define GSFAILURE_PARAMETER _In_ uintptr_t stack_cookie
#else
    #error Unsupported architecture
#endif

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
__declspec(noreturn) void __cdecl __report_gsfailure(GSFAILURE_PARAMETER)
{
    __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE);
}
#pragma warning(pop)



// Reports a specific security failure condition.  The type of failure that
// occurred is embodied in the failure_code that is provided as a parameter.
// A specific failure condition can optionally specify additional parameters.
__declspec(noreturn) void __cdecl __report_securityfailureEx(
    _In_                            ULONG  failure_code,
    _In_                            ULONG  parameter_count,
    _In_reads_opt_(parameter_count) void** parameters
    )
{
    UNREFERENCED_PARAMETER(parameter_count);
    UNREFERENCED_PARAMETER(parameters);

    __fastfail(failure_code);
}



// Reports a specific security failure condition.  The type of failure that
// occurred is embodied in the failure_code that is provided as a parameter.
// If a failure condition needs to specify additional parameters then it
// should call __report_securityfailureEx instead.
__declspec(noreturn) void __cdecl __report_securityfailure(ULONG failure_code)
{
    __fastfail(failure_code);
}



// Declare stub for rangecheckfailure, since these occur often enough that the
// code bloat of setting up the parameters hurts performance
__declspec(noreturn) void __cdecl __report_rangecheckfailure()
{
    __fastfail(FAST_FAIL_RANGE_CHECK_FAILURE);
}
