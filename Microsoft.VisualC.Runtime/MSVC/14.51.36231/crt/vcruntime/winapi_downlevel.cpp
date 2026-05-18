//
// winapi_downlevel.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of wrappers for Windows API functions that previously could not be called
// directly, but are now available on all supported operating systems.
//
#include <vcruntime_internal.h>

// TRANSITION, ABI: __vcrt_InitializeCriticalSectionEx() is preserved for binary compatibility
extern "C" BOOL __cdecl __vcrt_InitializeCriticalSectionEx(
    _Out_ LPCRITICAL_SECTION const critical_section,
    _In_  DWORD              const spin_count,
    _In_  DWORD              const flags
    )
{
    return InitializeCriticalSectionEx(critical_section, spin_count, flags);
}
