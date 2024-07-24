//
// sys_main.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The DriverEntry(...) entry point, linked into client executables that
// uses DriverMain(...).
//

#define _MUSA_SCRT_BUILD_PGO_INITIALIZER
#include "sys_common.inl"


extern"C" NTSTATUS NTAPI DriverMain (_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern"C" NTSTATUS NTAPI DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    return __scrt_common_main_seh(DriverObject, RegistryPath, DriverMain);
}
