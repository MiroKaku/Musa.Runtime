//
// initialization.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This file defines the main initialization and uninitialization routines for
// the VCRuntime, shared by both the static and dynamic VCRuntime libraries.  In
// the dynamic VCRuntime library, these are called by DllMain.  In the static
// VCRuntime library, these are called by the initialization code.
//
#include <vcruntime_internal.h>

extern "C" {

#if !defined(_CRT_WINDOWS)

#if defined(_M_ARM)
#define INIT_STRING_FUNCTIONS 1

    // These functions are used to set the default behaviour of memcpy operations
    // on ARM.  If an external cache is available, memcpy() uses the *_integer
    // functions.  Otherwise, memcpy() will keep the defaults, which are the
    // *_neon functions.
    typedef void * _memcpy_t(void *, const void *, size_t);

    extern _memcpy_t * __memcpy_forward_large_func;
    extern _memcpy_t * __memcpy_reverse_large_func;
    extern _memcpy_t __memcpy_forward_large_integer;
    extern _memcpy_t __memcpy_reverse_large_integer;

    // Update memcpy function pointers:
    static void __cdecl init_string_functions()
    {
        if (IsProcessorFeaturePresent(PF_ARM_EXTERNAL_CACHE_AVAILABLE))
        {
            /*
             * Set memcpy() to use the integer functions; overriding
             * the defaults, which are the NEON functions.
             */
            __memcpy_forward_large_func = &__memcpy_forward_large_integer;
            __memcpy_reverse_large_func = &__memcpy_reverse_large_integer;
        }
    }

#elif (defined(_M_ARM64) || defined(_M_ARM64EC) || defined(_M_HYBRID_X86_ARM64)) && \
      !defined(_CRT_APP) && !defined(_CRT_ONECORE) && !defined(_CRT_ENCLAVE)

#define INIT_STRING_FUNCTIONS 1

    int __arm64_safe_unaligned_memory_access;

    static
    VERSIONHELPERAPI
    __IsWindowsVersionOrGreaterEx(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor, WORD wBuildNumber)
    {
        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        DWORDLONG        const dwlConditionMask = VerSetConditionMask(
            VerSetConditionMask(
            VerSetConditionMask(
            VerSetConditionMask(
                0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                VER_MINORVERSION, VER_GREATER_EQUAL),
                VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL),
                VER_BUILDNUMBER, VER_GREATER_EQUAL);

        osvi.dwMajorVersion = wMajorVersion;
        osvi.dwMinorVersion = wMinorVersion;
        osvi.wServicePackMajor = wServicePackMajor;
        osvi.dwBuildNumber = wBuildNumber;

        return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
    }

    static
    VERSIONHELPERAPI
    __IsWindows1122H2OrGreater()
    {
        const WORD WIN11_22H2_BUILD_NUM = 22621;
        return __IsWindowsVersionOrGreaterEx(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0, WIN11_22H2_BUILD_NUM);
    }

    static void __cdecl init_string_functions()
    {
        if (__IsWindows1122H2OrGreater())
        {
            __arm64_safe_unaligned_memory_access = 1;
        }
    }

#endif
#endif

#if !defined(INIT_STRING_FUNCTIONS)

    static void __cdecl init_string_functions()
    {
        // No-op on non-ARM platforms
    }

#endif



bool __cdecl __vcrt_initialize()
{
    #if defined CRTDLL
    __isa_available_init();
    #endif

    init_string_functions();

    if (!__vcrt_initialize_locks())
    {
        return false;
    }

    if (!__vcrt_initialize_ptd())
    {
        __vcrt_uninitialize_locks();
        return false;
    }

    return true;
}

bool __cdecl __vcrt_uninitialize(_In_ bool const terminating)
{
    UNREFERENCED_PARAMETER(terminating);

    // If the process is terminating, there's no point in cleaning up, except
    // in debug builds.
    #ifndef _DEBUG
    if (!terminating)
    #endif
    {
        __vcrt_uninitialize_ptd();
        __vcrt_uninitialize_locks();
    }

    return true;
}

bool __cdecl __vcrt_uninitialize_critical()
{
    __vcrt_uninitialize_ptd();

    return true;
}

bool __cdecl __vcrt_thread_attach()
{
    // Create a per-thread data structure for this thread (getptd will attempt
    // to create a new per-thread data structure if one does not already exist
    // for this thread):
    if (__vcrt_getptd_noexit() == nullptr)
        return false;

    return true;
}

bool __cdecl __vcrt_thread_detach()
{
    // Free the per-thread data structure for this thread:
    __vcrt_freeptd_for_this_thread();
    return true;
}

}
