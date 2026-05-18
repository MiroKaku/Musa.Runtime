//
// utility_desktop.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// CRT startup and termination functionality specific to use of the CRT in a
// Desktop app.
//
#include <vcstartup_internal.h>
#include <eh.h>
#include <ehdata.h>
#include <intrin.h>
#include <trnsctrl.h>


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Startup Support
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" WORD __cdecl __scrt_get_show_window_mode()
{
    STARTUPINFOW startup_info{};
    GetStartupInfoW(&startup_info);

    return startup_info.dwFlags & STARTF_USESHOWWINDOW
        ? startup_info.wShowWindow
        : SW_SHOWDEFAULT;
}

extern "C" bool __cdecl __scrt_is_managed_app()
{
    PIMAGE_DOS_HEADER const dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(GetModuleHandleW(nullptr));
    if (dos_header == nullptr)
        return false;

    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    PIMAGE_NT_HEADERS const pe_header = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<BYTE*>(dos_header) + dos_header->e_lfanew);

    if (pe_header->Signature != IMAGE_NT_SIGNATURE)
        return false;

    if (pe_header->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        return false;

    // prefast assumes we are overrunning __ImageBase
    #pragma warning(push)
    #pragma warning(disable: 26000)

    if (pe_header->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
        return false;

    #pragma warning(pop)

    if (pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress == 0)
        return false;

    return true;
}

extern "C" int __cdecl __scrt_initialize_winrt()
{
    // We never initialize WinRT in a Desktop app:
    return 0;
}

// If exe_initialize_mta.lib doesn't get linked in, then the "real" function won't be seen, so all we do is call the no-op stub.
extern "C" int __cdecl __scrt_exe_initialize_mta();

extern "C" int __cdecl __scrt_stub_for_initialize_mta()
{
    // By default, we don't initialize a COM MTA
    return 0;
}

_VCRT_DECLARE_ALTERNATE_NAME(__scrt_exe_initialize_mta, __scrt_stub_for_initialize_mta)

extern "C" int __cdecl __scrt_initialize_mta()
{
    return __scrt_exe_initialize_mta();
}

extern "C" LONG WINAPI __scrt_unhandled_exception_filter(LPEXCEPTION_POINTERS const pointers)
{
    auto const exception_record = reinterpret_cast<EHExceptionRecord*>(pointers->ExceptionRecord);
    if (PER_IS_MSVC_PURE_OR_NATIVE_EH(exception_record))
    {
        _pCurrentException = reinterpret_cast<EHExceptionRecord*>(exception_record);
        _pCurrentExContext = pointers->ContextRecord;
        terminate();
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

extern "C" void __cdecl __scrt_set_unhandled_exception_filter()
{
    SetUnhandledExceptionFilter(__scrt_unhandled_exception_filter);
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Fatal Error Reporting
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// There are parts of the runtime startup process that absolutely must succeed.
// If an error occurs during these parts of the startup process, we terminate
// the process by invoking __fastfail.
//
// This functionality is equivalent to the fault-reporting logic in the AppCRT.
// We cannot make use of the AppCRT fault-reporting logic here, because we may
// encounter a fatal error before we have fully initialized the AppCRT.

#if defined(_M_IX86) || defined(_M_X64)

extern "C" int __scrt_debugger_hook_flag = 0;

extern "C" __declspec(noinline) void __cdecl _CRT_DEBUGGER_HOOK(int const reserved)
{
    UNREFERENCED_PARAMETER(reserved);

    // We assign zero to the debugger hook flag so that this function is not
    // folded when optimized.  The flag is not otherwise used.
    __scrt_debugger_hook_flag = 0;
}

#endif

extern "C" void __cdecl __scrt_fastfail(unsigned const code)
{
    // The __fastfail intrinsic is always available on our supported operating systems:
    __fastfail(code);
}
