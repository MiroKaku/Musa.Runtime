//
// wcscat.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcscat(), which concatenates (appends) a copy of the source string to
// the end of the destination string.
//
// This function assumes that the destination buffer is sufficiently large to
// store the appended string.
//
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>



#if defined _M_X64 || defined _M_IX86 || defined _M_ARM || defined _M_ARM64
    #pragma warning(disable:4163)
    #pragma function(wcscat)
    #pragma function(wcslen)
    #pragma function(wcscpy)
#endif



extern "C" wchar_t * __cdecl wcscat(
    wchar_t*       const destination,
    wchar_t const*       source
    )

{
    wcscpy(destination + wcslen(destination), source);
    return destination;
}
