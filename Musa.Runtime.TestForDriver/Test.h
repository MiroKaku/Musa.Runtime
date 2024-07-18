#pragma once
#include <kext/kallocator.h>

#include <vector>
#include <functional>

/////////////////////////////////////////////////////////////

#ifndef ASSERT
#  define ASSERT assert
#endif

// Logging
#ifdef _DEBUG
#define MusaLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Musa.Runtime][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MusaLOG(...)
#pragma warning(disable: 4101 4189) // unreferenced local variable
#endif

// [[maybe_unused]] attributes on STL functions
#ifndef _HAS_MAYBE_UNUSED
#ifndef __has_cpp_attribute
#define _HAS_MAYBE_UNUSED 0
#elif __has_cpp_attribute(maybe_unused) >= 201603L // TRANSITION, VSO#939899 (need toolset update)
#define _HAS_MAYBE_UNUSED 1
#else
#define _HAS_MAYBE_UNUSED 0
#endif
#endif // _HAS_MAYBE_UNUSED

#if _HAS_MAYBE_UNUSED
#define _MAYBE_UNUSED [[maybe_unused]]
#else // ^^^ CAN HAZ [[maybe_unused]] / NO CAN HAZ [[maybe_unused]] vvv
#define _MAYBE_UNUSED
#endif // _HAS_MAYBE_UNUSED

/////////////////////////////////////////////////////////////

namespace UnitTest
{
    template<typename T>
    using  Vector = std::vector<T, std::kallocator<T, PagedPool, 'TSET'>>;

#if _MSVC_LANG < 201703L
    __declspec(selectany) extern Vector<std::function<void()>> TestVec;
#define TEST(name) TEST_ ## name
#define TEST_PUSH(name) static auto _VEIL_CONCATENATE(TEST_Push_, __COUNTER__) = ::UnitTest::TestVec.emplace(::UnitTest::TestVec.end(), TEST(name))

#else
    inline Vector<std::function<void()>> TestVec;
#define TEST(name) TEST_ ## name
#define TEST_PUSH(name) static auto _VEIL_CONCATENATE(TEST_Push_, __COUNTER__) = ::UnitTest::TestVec.emplace_back(TEST(name))
#endif

}
