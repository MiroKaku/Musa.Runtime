//
// wcsncat.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcsncat(), which appends n characters of the source string onto the
// end of the destination string.  The destination string is always null
// terminated.  Unlike wcsncpy, this function does not pad out to 'count'
// characters.
//
#include <string.h>


#pragma warning(disable:__WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED) // 26018

#pragma function(memcpy, wcslen)

extern "C" wchar_t* __cdecl wcsncat(
    wchar_t*       const destination,
    wchar_t const* const source,
    size_t         const count
    )
{
    wchar_t *start = destination + wcslen(destination);
    size_t srclen = wcsnlen(source, count);
    memcpy(start, source, srclen * sizeof(wchar_t));
    start[srclen] = '\0';
    return(destination);
}
