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

#include <string.h>

#pragma function(strcpy)

/***
*char *strcpy(dst, src) - copy one string over another
*
*Purpose:
*       Copies the string src into the spot specified by
*       dest; assumes enough room.
*
*Entry:
*       char * dst - string over which "src" is to be copied
*       const char * src - string to be copied over "dst"
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*******************************************************************************/

char * __cdecl strcpy(char * dst, const char * src)
{
        char * cp = dst;

        while((*cp++ = *src++) != '\0')
                ;               /* Copy src over dst */

        return( dst );
}
