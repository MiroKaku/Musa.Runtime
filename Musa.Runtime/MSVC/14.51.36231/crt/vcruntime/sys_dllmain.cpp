//
// sys_dllmain.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The DllInitialize(...) entry point, linked into client executables that
// uses DllMainForAttach(...).
//

//
// Notice: Experimental features!
//

#include "sys_common.inl"


EXTERN_C NTSTATUS NTAPI DriverMainForDllAttach(_In_ PUNICODE_STRING RegistryPath);
EXTERN_C NTSTATUS NTAPI DriverMainForDllDetach();


extern"C" NTSTATUS NTAPI DllInitialize(_In_ PUNICODE_STRING RegistryPath)
{
    return __scrt_common_main_seh(nullptr, RegistryPath,
        [](auto, auto register_path){ return DriverMainForDllAttach(register_path); });
}

extern"C" NTSTATUS NTAPI DllUnload()
{
    auto const result = DriverMainForDllDetach();
    if (!NT_SUCCESS(result)) {
        return result;
    }

    _cexit();
    __scrt_uninitialize_crt(true, true);

    return MusaCoreShutdown();
}
