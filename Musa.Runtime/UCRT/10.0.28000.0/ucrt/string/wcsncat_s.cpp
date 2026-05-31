//
// wcsncat_s.cpp - kernel overlay
//
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

#include <corecrt_internal_string_templates.h>
#include <string.h>

extern "C" errno_t __cdecl wcsncat_s(
    wchar_t*       const destination,
    size_t         const size_in_elements,
    wchar_t const* const source,
    size_t         const count
    )
{
    return common_tcsncat_s(destination, size_in_elements, source, count);
}

#undef max
#undef min
