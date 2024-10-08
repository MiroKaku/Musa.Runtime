﻿#pragma once

//
// libcmt[d].lib        -> Musa.Runtime.CRT
// libcpmt[d].lib       -> Musa.Runtime.STL
// libvcruntime[d].lib  -> Musa.Runtime.VCRT
// libucrt[d].lib       -> Musa.Runtime.UCRT
//

#pragma warning(disable: 4005 4189 4245 4457 4499 4838)

// System Header
#define _NO_CRT_STDIO_INLINE    // TODO: NLS
#define _CRT_SECURE_NO_WARNINGS

#include <corecrt.h>

#undef _CRT_NO_TIME_T
#ifdef _USE_32BIT_TIME_T
typedef __time32_t time_t;
#else
typedef __time64_t time_t;
#endif

#define _ATOMIC_WAIT_ON_ADDRESS_STATICALLY_AVAILABLE 0

#define NOMINMAX
#include <Veil.h>

// C/C++  Header

// Local  Header

// Global Variable

// Global Macro
#define __WARNING_NOT_SATISFIED     28020
#define __WARNING_UNUSED_ASSIGNMENT 28931
