/***
*strcat.c - contains strcat() and strcpy()
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Strcpy() copies one string onto another.
*
*       Strcat() concatenates (appends) a copy of the source string to the
*       end of the destination string, returning the destination string.
*
*******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#pragma function(strcat)
#pragma function(strlen)
#pragma function(strcpy)

/***
*char *strcat(dst, src) - concatenate (append) one string to another
*
*Purpose:
*       Concatenates src onto the end of dest.  Assumes enough
*       space in dest.
*
*Entry:
*       char *dst - string to which "src" is to be appended
*       const char *src - string to be appended to the end of "dst"
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl strcat (
        char * dst,
        const char * src
        )
{
        strcpy(dst + strlen(dst), src);

        return dst;
}
