//
// sys_common.inl
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The implementation of the common executable entry point code.  There are four
// executable entry points defined by the CRT, one for each of the user-definable
// entry points:
//
//  * DriverEntry     => DriverMain
//  * DllInitialize   => DriverMainForDllAttach
//
// These functions all behave the same, except for which user-definable main
// function they call and whether they accumulate and pass narrow or wide string
// arguments.  This file contains the common code shared by all four of those
// entry points.
//
// The actual entry points are defined in four .cpp files alongside this .inl
// file.  At most one of these .cpp files will be linked into the resulting
// executable, so we can treat this .inl file as if its contents are only linked
// into the executable once as well.
//
#include <vcstartup_internal.h>
#include <vcruntime_internal.h>
#include <new.h>
#include <stdlib.h>

#include <Musa.Core.h>


#define __scrt_module_type_sys ((__scrt_module_type)3)

EXTERN_C
_ACRTIMP int __cdecl _seh_filter_sys(
    _In_ unsigned long       ExceptionNum,
    _In_ PEXCEPTION_POINTERS ExceptionPtr
);


#ifdef _MUSA_SCRT_BUILD_PGO_INITIALIZER

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// C/C++ Initializer
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

static int __cdecl pre_c_initialization()
{
    if (!__scrt_initialize_onexit_tables(__scrt_module_type_sys))
        __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);

    #ifdef _M_IX86
    // Clear the x87 exception flags.  Any other floating point initialization
    // should already have taken place before this function is called.
    _asm { fnclex }
    #endif

    __scrt_initialize_type_info();

    _initialize_invalid_parameter_handler();
    _initialize_denormal_control();

    #ifdef _M_IX86
    _initialize_default_precision();
    #endif

    return 0;
}

static void __cdecl pre_cpp_initialization()
{
    // Before we begin C++ initialization, set the unhandled exception
    // filter so that unhandled C++ exceptions result in std::terminate
    // being called:
    __scrt_set_unhandled_exception_filter();

    _set_new_mode(_get_startup_new_mode());
}

// When both the PGO instrumentation library and the CRT are statically linked,
// PGO will initialize itself in XIAB.  We do most pre-C initialization before
// PGO is initialized, but defer some initialization steps to after.  See the
// commentary in post_pgo_initialization for details.
extern"C" _CRTALLOC(".CRT$XIAA") _PIFV pre_c_initializer    = pre_c_initialization;
extern"C" _CRTALLOC(".CRT$XCAA") _PVFV pre_cpp_initializer  = pre_cpp_initialization;

#endif



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Common DriverEntry()/DllInitialize() implementation
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

static PDRIVER_UNLOAD __scrt_drv_unload = nullptr;
static __declspec(noinline) VOID NTAPI __scrt_common_exit(_In_ PDRIVER_OBJECT driver_object)
{
    if (__scrt_drv_unload) {
        __scrt_drv_unload(driver_object);
    }

    _cexit();
    __scrt_uninitialize_crt(true, true);

    (void)MusaCoreShutdown();
}

static __declspec(noinline) int __cdecl __scrt_common_main_seh(
    _In_opt_ PDRIVER_OBJECT     driver_object,
    _In_     PUNICODE_STRING    registry_path,
    _In_     PDRIVER_INITIALIZE driver_main
    )
{
    auto main_status = MusaCoreStartup(driver_object, registry_path);
    if (!NT_SUCCESS(main_status)) {
        return main_status;
    }

    if (!__scrt_initialize_crt(__scrt_module_type_sys))
        __scrt_fastfail(FAST_FAIL_FATAL_APP_EXIT);

    __try {
        if (_initterm_e(__xi_a, __xi_z) != 0)
            return STATUS_DRIVER_INTERNAL_ERROR;

        _initterm(__xc_a, __xc_z);

        // If this module has any dynamically initialized __declspec(thread)
        // variables, then we invoke their initialization for the primary thread
        // used to start the process:
        _tls_callback_type const* const tls_init_callback = __scrt_get_dyn_tls_init_callback();
        if (*tls_init_callback != nullptr && __scrt_is_nonwritable_in_current_image(tls_init_callback)) {
            (*tls_init_callback)(nullptr, DLL_THREAD_ATTACH, nullptr);
        }

        // If this module has any thread-local destructors, register the
        // callback function with the Unified CRT to run on exit.
        _tls_callback_type const* const tls_dtor_callback = __scrt_get_dyn_tls_dtor_callback();
        if (*tls_dtor_callback != nullptr && __scrt_is_nonwritable_in_current_image(tls_dtor_callback)) {
            _register_thread_local_exe_atexit_callback(*tls_dtor_callback);
        }

        //
        // Initialization is complete; invoke main...
        //

        auto const main_result = driver_main(driver_object, registry_path);
        if (NT_SUCCESS(main_result)) {
            if (driver_object) {
                __scrt_drv_unload = static_cast<PDRIVER_UNLOAD>(InterlockedExchangePointer(
                    reinterpret_cast<PVOID volatile*>(&driver_object->DriverUnload), &__scrt_common_exit));
            }
        }
        else {
            _cexit();

            // We terminate the CRT:
            __scrt_uninitialize_crt(true, false);

            (void)MusaCoreShutdown();
        }

        return main_result;
    }
    __except (_seh_filter_sys(GetExceptionCode(), GetExceptionInformation())) {
        // Note:  We should never reach this except clause.
        int const main_result = GetExceptionCode();

        _exit(main_result);
    }
}
