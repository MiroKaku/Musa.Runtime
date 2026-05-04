//
// dbgstubs.cpp -- Release-mode debug reporting stubs
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// In Release builds, the debug CRT files (dbgrpt.cpp, dbgrptt.cpp) are excluded.
// Math library objects (contrlfp, fpexcept) still reference _CrtDbgReportW.
//

#include <corecrt_internal.h>

#ifdef NDEBUG
extern "C" int __cdecl _CrtDbgReportW(int, const wchar_t*, int, const wchar_t*, const wchar_t*, ...)
{
    return 0;
}
#endif
