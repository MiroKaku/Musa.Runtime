//
// locale_stubs.cpp - kernel overlay: stub locale construction functions not
// needed for C locale (format/regex only use default C locale).
//
#include <yvals.h>
#include <xlocale>

_STD_BEGIN

// Stub _Makexloc — C locale doesn't have extended locales
void __CLRCALL_PURE_OR_CDECL locale::_Locimp::_Makexloc(
    const _Locinfo&, int, locale::_Locimp*, const locale*)
{
}

_STD_END

// Provide _Makewloc stub (wide-char locale construction)
_STD_BEGIN
void __CLRCALL_PURE_OR_CDECL locale::_Locimp::_Makewloc(
    const _Locinfo&, int, locale::_Locimp*, const locale*)
{
}
_STD_END

// Provide _Makeushloc stub (uchar locale construction)
_STD_BEGIN
void __CLRCALL_PURE_OR_CDECL locale::_Locimp::_Makeushloc(
    const _Locinfo&, int, locale::_Locimp*, const locale*)
{
}
_STD_END
