#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"
#include "LLVM.TestSuite/make_test_thread.h"
#include "LLVM.TestSuite/cmpxchg_loop.h"


namespace
{
    template <class A, class T>
    void test_address_impl()
    {
        typedef typename std::remove_pointer<T>::type X;
        A obj(T(0));
        assert(obj == T(0));
        {
            bool lockfree = obj.is_lock_free();
            (void)lockfree;
        #if TEST_STD_VER >= 17
            if (A::is_always_lock_free)
                assert(lockfree);
        #endif
        }
        obj.store(T(0));
        assert(obj == T(0));
        obj.store(T(1), std::memory_order_release);
        assert(obj == T(1));
        assert(obj.load() == T(1));
        assert(obj.load(std::memory_order_acquire) == T(1));
        assert(obj.exchange(T(2)) == T(1));
        assert(obj == T(2));
        assert(obj.exchange(T(3), std::memory_order_relaxed) == T(2));
        assert(obj == T(3));
        T x = obj;
        assert(cmpxchg_weak_loop(obj, x, T(2)) == true);
        assert(obj == T(2));
        assert(x == T(3));
        assert(obj.compare_exchange_weak(x, T(1)) == false);
        assert(obj == T(2));
        assert(x == T(2));
        x = T(2);
        assert(obj.compare_exchange_strong(x, T(1)) == true);
        assert(obj == T(1));
        assert(x == T(2));
        assert(obj.compare_exchange_strong(x, T(0)) == false);
        assert(obj == T(1));
        assert(x == T(1));
        assert((obj = T(0)) == T(0));
        assert(obj == T(0));
        obj = T(2 * sizeof(X));
        assert((obj += std::ptrdiff_t(3)) == T(5 * sizeof(X)));
        assert(obj == T(5 * sizeof(X)));
        assert((obj -= std::ptrdiff_t(3)) == T(2 * sizeof(X)));
        assert(obj == T(2 * sizeof(X)));

        {
            TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {23};
            A& zero = *new (storage) A();
            assert(zero == T(0));
            zero.~A();
        }
    }

    template <class A, class T>
    void test_address()
    {
        test_address_impl<A, T>();
        test_address_impl<volatile A, T>();
    }
}

TEST(address)
{
    test_address<std::atomic<int*>, int*>();
}

TEST(bool)
{
    {
        volatile std::atomic<bool> obj(true);
        assert(obj == true);
        {
            bool lockfree = obj.is_lock_free();
            (void)lockfree;
        #if TEST_STD_VER >= 17
            if (std::atomic<bool>::is_always_lock_free)
                assert(lockfree);
        #endif
        }
        obj.store(false);
        assert(obj == false);
        obj.store(true, std::memory_order_release);
        assert(obj == true);
        assert(obj.load() == true);
        assert(obj.load(std::memory_order_acquire) == true);
        assert(obj.exchange(false) == true);
        assert(obj == false);
        assert(obj.exchange(true, std::memory_order_relaxed) == false);
        assert(obj == true);
        bool x = obj;
        assert(cmpxchg_weak_loop(obj, x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_weak(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        obj.store(true);
        x = true;
        assert(cmpxchg_weak_loop(obj, x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_strong(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        assert((obj = false) == false);
        assert(obj == false);
        assert((obj = true) == true);
        assert(obj == true);
    }
    {
        std::atomic<bool> obj(true);
        assert(obj == true);
        {
            bool lockfree = obj.is_lock_free();
            (void)lockfree;
        #if TEST_STD_VER >= 17
            if (std::atomic<bool>::is_always_lock_free)
                assert(lockfree);
        #endif
        }
        obj.store(false);
        assert(obj == false);
        obj.store(true, std::memory_order_release);
        assert(obj == true);
        assert(obj.load() == true);
        assert(obj.load(std::memory_order_acquire) == true);
        assert(obj.exchange(false) == true);
        assert(obj == false);
        assert(obj.exchange(true, std::memory_order_relaxed) == false);
        assert(obj == true);
        bool x = obj;
        assert(cmpxchg_weak_loop(obj, x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_weak(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        obj.store(true);
        x = true;
        assert(cmpxchg_weak_loop(obj, x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_strong(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        assert((obj = false) == false);
        assert(obj == false);
        assert((obj = true) == true);
        assert(obj == true);
    }
    {
        std::atomic_bool obj(true);
        assert(obj == true);
        {
            bool lockfree = obj.is_lock_free();
            (void)lockfree;
        #if TEST_STD_VER >= 17
            if (std::atomic_bool::is_always_lock_free)
                assert(lockfree);
        #endif
        }
        obj.store(false);
        assert(obj == false);
        obj.store(true, std::memory_order_release);
        assert(obj == true);
        assert(obj.load() == true);
        assert(obj.load(std::memory_order_acquire) == true);
        assert(obj.exchange(false) == true);
        assert(obj == false);
        assert(obj.exchange(true, std::memory_order_relaxed) == false);
        assert(obj == true);
        bool x = obj;
        assert(cmpxchg_weak_loop(obj, x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_weak(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        obj.store(true);
        x = true;
        assert(cmpxchg_weak_loop(obj, x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false) == true);
        assert(obj == false);
        assert(x == true);
        assert(obj.compare_exchange_strong(x, true,
            std::memory_order_seq_cst) == false);
        assert(obj == false);
        assert(x == false);
        x = true;
        obj.store(true);
        assert(obj.compare_exchange_strong(x, false,
            std::memory_order_seq_cst,
            std::memory_order_seq_cst) == true);
        assert(obj == false);
        assert(x == true);
        assert((obj = false) == false);
        assert(obj == false);
        assert((obj = true) == true);
        assert(obj == true);
    }
    {
        typedef std::atomic<bool> A;
        TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {1};
        A& zero = *new (storage) A();
        assert(zero == false);
        zero.~A();
    }
}

namespace
{
    template <class A, class T>
    void test_integral_impl()
    {
        A obj(T(0));
        assert(obj == T(0));
        {
            bool lockfree = obj.is_lock_free();
            (void)lockfree;
        #if TEST_STD_VER >= 17
            if (A::is_always_lock_free)
                assert(lockfree);
        #endif
        }
        obj.store(T(0));
        assert(obj == T(0));
        obj.store(T(1), std::memory_order_release);
        assert(obj == T(1));
        assert(obj.load() == T(1));
        assert(obj.load(std::memory_order_acquire) == T(1));
        assert(obj.exchange(T(2)) == T(1));
        assert(obj == T(2));
        assert(obj.exchange(T(3), std::memory_order_relaxed) == T(2));
        assert(obj == T(3));
        T x = obj;
        assert(cmpxchg_weak_loop(obj, x, T(2)) == true);
        assert(obj == T(2));
        assert(x == T(3));
        assert(obj.compare_exchange_weak(x, T(1)) == false);
        assert(obj == T(2));
        assert(x == T(2));
        x = T(2);
        assert(obj.compare_exchange_strong(x, T(1)) == true);
        assert(obj == T(1));
        assert(x == T(2));
        assert(obj.compare_exchange_strong(x, T(0)) == false);
        assert(obj == T(1));
        assert(x == T(1));
        assert((obj = T(0)) == T(0));
        assert(obj == T(0));
        assert(obj++ == T(0));
        assert(obj == T(1));
        assert(++obj == T(2));
        assert(obj == T(2));
        assert(--obj == T(1));
        assert(obj == T(1));
        assert(obj-- == T(1));
        assert(obj == T(0));
        obj = T(2);
        assert((obj += T(3)) == T(5));
        assert(obj == T(5));
        assert((obj -= T(3)) == T(2));
        assert(obj == T(2));
        assert((obj |= T(5)) == T(7));
        assert(obj == T(7));
        assert((obj &= T(0xF)) == T(7));
        assert(obj == T(7));
        assert((obj ^= T(0xF)) == T(8));
        assert(obj == T(8));

        {
            TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {23};
            A& zero = *new (storage) A();
            assert(zero == 0);
            zero.~A();
        }
    }

    template <class A, class T>
    void test_integral()
    {
        test_integral_impl<A, T>();
        test_integral_impl<volatile A, T>();
    }
}

TEST(integral)
{
    test_integral<std::atomic_char, char>();
    test_integral<std::atomic_schar, signed char>();
    test_integral<std::atomic_uchar, unsigned char>();
    test_integral<std::atomic_short, short>();
    test_integral<std::atomic_ushort, unsigned short>();
    test_integral<std::atomic_int, int>();
    test_integral<std::atomic_uint, unsigned int>();
    test_integral<std::atomic_long, long>();
    test_integral<std::atomic_ulong, unsigned long>();
    test_integral<std::atomic_llong, long long>();
    test_integral<std::atomic_ullong, unsigned long long>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test_integral<std::atomic_char8_t, char8_t>();
#endif
    test_integral<std::atomic_char16_t, char16_t>();
    test_integral<std::atomic_char32_t, char32_t>();
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    test_integral<std::atomic_wchar_t, wchar_t>();
#endif

    test_integral<std::atomic_int8_t, int8_t>();
    test_integral<std::atomic_uint8_t, uint8_t>();
    test_integral<std::atomic_int16_t, int16_t>();
    test_integral<std::atomic_uint16_t, uint16_t>();
    test_integral<std::atomic_int32_t, int32_t>();
    test_integral<std::atomic_uint32_t, uint32_t>();
    test_integral<std::atomic_int64_t, int64_t>();
    test_integral<std::atomic_uint64_t, uint64_t>();

    test_integral<volatile std::atomic_char, char>();
    test_integral<volatile std::atomic_schar, signed char>();
    test_integral<volatile std::atomic_uchar, unsigned char>();
    test_integral<volatile std::atomic_short, short>();
    test_integral<volatile std::atomic_ushort, unsigned short>();
    test_integral<volatile std::atomic_int, int>();
    test_integral<volatile std::atomic_uint, unsigned int>();
    test_integral<volatile std::atomic_long, long>();
    test_integral<volatile std::atomic_ulong, unsigned long>();
    test_integral<volatile std::atomic_llong, long long>();
    test_integral<volatile std::atomic_ullong, unsigned long long>();
    test_integral<volatile std::atomic_char16_t, char16_t>();
    test_integral<volatile std::atomic_char32_t, char32_t>();
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    test_integral<volatile std::atomic_wchar_t, wchar_t>();
#endif

    test_integral<volatile std::atomic_int8_t, int8_t>();
    test_integral<volatile std::atomic_uint8_t, uint8_t>();
    test_integral<volatile std::atomic_int16_t, int16_t>();
    test_integral<volatile std::atomic_uint16_t, uint16_t>();
    test_integral<volatile std::atomic_int32_t, int32_t>();
    test_integral<volatile std::atomic_uint32_t, uint32_t>();
    test_integral<volatile std::atomic_int64_t, int64_t>();
    test_integral<volatile std::atomic_uint64_t, uint64_t>();
}

#endif
