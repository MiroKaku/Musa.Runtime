//
// winapi_thunks.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of wrappers for Windows API functions that cannot be called
// directly because they are not available on all supported operating systems.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsecapi.h>
#include <corecrt_internal.h>


extern "C" DWORD WINAPI __acrt_FlsAlloc(PFLS_CALLBACK_FUNCTION const callback)
{
    return FlsAlloc(callback);
}

extern "C" BOOL WINAPI __acrt_FlsFree(DWORD const fls_index)
{
    return FlsFree(fls_index);
}

extern "C" PVOID WINAPI __acrt_FlsGetValue(DWORD const fls_index)
{
    return FlsGetValue(fls_index);
}

extern "C"
DECLSPEC_GUARDNOCF
PVOID WINAPI __acrt_FlsGetValue2(DWORD const fls_index)
{
#if (NTDDI_VERSION >= NTDDI_WIN11_GE)
    return FlsGetValue2(fls_index);
#else
    return FlsGetValue(fls_index);
#endif
}

extern "C" BOOL WINAPI __acrt_FlsSetValue(DWORD const fls_index, PVOID const fls_data)
{
    return FlsSetValue(fls_index, fls_data);
}

extern "C" BOOL WINAPI __acrt_IsThreadAFiber()
{
    return false;
}

extern "C" BOOL WINAPI __acrt_InitializeCriticalSectionEx(
    LPCRITICAL_SECTION const critical_section,
    DWORD              const spin_count,
    DWORD              const flags
)
{
    return InitializeCriticalSectionEx(critical_section, spin_count, flags);
}

extern "C" BOOLEAN WINAPI __acrt_RtlGenRandom(
    PVOID const buffer,
    ULONG const buffer_count
)
{
    return RtlGenRandom(buffer, buffer_count);
}

// ---- UCRT delegates to kernel32 thunks (round 2) ----

// __acrt_GetSystemTimePreciseAsFileTime — delegate to GetSystemTimeAsFileTime
extern "C" void __cdecl __acrt_GetSystemTimePreciseAsFileTime(LPFILETIME const lpTime)
{
    GetSystemTimeAsFileTime(lpTime);
}

// __acrt_AreFileApisANSI — always FALSE in kernel mode
extern "C" int __cdecl __acrt_AreFileApisANSI()
{
    return FALSE;
}

// __acrt_SetEnvironmentVariableA — ANSI→WideChar→SetEnvironmentVariableW
extern "C" int __cdecl __acrt_SetEnvironmentVariableA(LPCSTR const lpName, LPCSTR const lpValue)
{
    if (!lpName) return 0;

    int const cchName = MultiByteToWideChar(CP_ACP, 0, lpName, -1, nullptr, 0);
    int const cchValue = lpValue ? MultiByteToWideChar(CP_ACP, 0, lpValue, -1, nullptr, 0) : 0;

    auto const wName = static_cast<LPWSTR>(_malloca(cchName * sizeof(WCHAR)));
    auto const wValue = lpValue ? static_cast<LPWSTR>(_malloca(cchValue * sizeof(WCHAR))) : nullptr;

    if (!wName || (lpValue && !wValue))
    {
        _freea(wName);
        _freea(wValue);
        RtlSetLastWin32Error(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    MultiByteToWideChar(CP_ACP, 0, lpName, -1, wName, cchName);
    if (wValue) MultiByteToWideChar(CP_ACP, 0, lpValue, -1, wValue, cchValue);

    BOOL const result = SetEnvironmentVariableW(wName, wValue);

    _freea(wName);
    _freea(wValue);

    return result;
}

// __acrt_GetDateFormatEx — delegate to GetDateFormatEx
extern "C" int __cdecl __acrt_GetDateFormatEx(
    LPCWSTR const lpLocaleName,
    DWORD const dwFlags,
    SYSTEMTIME const* lpDate,
    LPCWSTR const lpFormat,
    LPWSTR const lpDateStr,
    int const cchDate,
    LPCWSTR const lpCalendar)
{
    UNREFERENCED_PARAMETER(lpCalendar);
    return GetDateFormatEx(lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate, nullptr);
}

// __acrt_GetTimeFormatEx — delegate to GetTimeFormatEx
extern "C" int __cdecl __acrt_GetTimeFormatEx(
    LPCWSTR const lpLocaleName,
    DWORD const dwFlags,
    SYSTEMTIME const* lpTime,
    LPCWSTR const lpFormat,
    LPWSTR const lpTimeStr,
    int const cchTime)
{
    return GetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
}
