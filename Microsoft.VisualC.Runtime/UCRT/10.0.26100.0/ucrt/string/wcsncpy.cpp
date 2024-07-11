//
// wcsncpy.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcsncpy(), which copies a string from one buffer to another.  This
// function copies at most 'count' characters.  If fewer than 'count' characters
// are copied, the rest of the buffer is padded with null characters.
//
#include <string.h>

#pragma warning(disable:__WARNING_POSTCONDITION_NULLTERMINATION_VIOLATION) // 26036


#ifdef _M_ARM
    #pragma function(wcsncpy)
#endif

#pragma function(memcpy, memset)

extern "C" wchar_t * __cdecl wcsncpy(
    wchar_t*       const destination,
    wchar_t const* const source,
    size_t         const count
    )
{
    size_t srclen = wcsnlen(source, count);
    memcpy(destination, source, srclen * sizeof(wchar_t));
    if (srclen < count)
    {
        memset(destination + srclen, 0, (count - srclen) * sizeof(wchar_t));
    }
    return(destination);
}
