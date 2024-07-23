#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"

#undef  assert
#define assert ASSERT

#define TEST_HAS_NO_THREADS

#pragma warning(disable: 4310)

namespace
{
    template <class T>
    bool equals(T x, T y)
    {
        return x == y;
    }

    template <class T>
    T make_value(int i)
    {
        assert(i == 0 || i == 1);
        if constexpr (std::is_pointer_v<T>) {
            // So that pointers returned can be subtracted from one another
            static std::remove_const_t<std::remove_pointer_t<T>> d[2];
            return &d[i];
        }
        else {
            return T(i);
        }
    }

    // Test that all threads see the exact same sequence of events
    // Test will pass 100% if store_op and load_op are correctly
    // affecting the memory with seq_cst order
    template <class T, class StoreOp, class LoadOp>
    void test_seq_cst(StoreOp store_op, LoadOp load_op)
    {
    #ifndef TEST_HAS_NO_THREADS
        for (int i = 0; i < 100; ++i) {
            T old_value(make_value<T>(0));
            T new_value(make_value<T>(1));

            T copy_x = old_value;
            std::atomic_ref<T> const x(copy_x);
            T copy_y = old_value;
            std::atomic_ref<T> const y(copy_y);

            std::atomic_bool x_updated_first(false);
            std::atomic_bool y_updated_first(false);

            auto t1 = support::make_test_thread([&]
            {
                store_op(x, old_value, new_value);
            });

            auto t2 = support::make_test_thread([&]
            {
                store_op(y, old_value, new_value);
            });

            auto t3 = support::make_test_thread([&]
            {
                while (!equals(load_op(x), new_value)) {
                    std::this_thread::yield();
                }
                if (!equals(load_op(y), new_value)) {
                    x_updated_first.store(true, std::memory_order_relaxed);
                }
            });

            auto t4 = support::make_test_thread([&]
            {
                while (!equals(load_op(y), new_value)) {
                    std::this_thread::yield();
                }
                if (!equals(load_op(x), new_value)) {
                    y_updated_first.store(true, std::memory_order_relaxed);
                }
            });

            t1.join();
            t2.join();
            t3.join();
            t4.join();
            // thread 3 and thread 4 cannot see different orders of storing x and y
            assert(!(x_updated_first && y_updated_first));
        }
    #else
        (void)store_op;
        (void)load_op;
    #endif
    }

    // Test that all writes before the store are seen by other threads after the load
    // Test will pass 100% if store_op and load_op are correctly
    // affecting the memory with acquire-release order
    template <class T, class StoreOp, class LoadOp>
    void test_acquire_release(StoreOp store_op, LoadOp load_op)
    {
    #ifndef TEST_HAS_NO_THREADS
        for (auto i = 0; i < 100; ++i) {
            T old_value(make_value<T>(0));
            T new_value(make_value<T>(1));

            T copy = old_value;
            std::atomic_ref<T> const at(copy);
            int non_atomic = 5;

            constexpr auto number_of_threads = 8;
            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);

            for (auto j = 0; j < number_of_threads; ++j) {
                threads.push_back(support::make_test_thread([&at, &non_atomic, load_op, new_value]
                {
                    while (!equals(load_op(at), new_value)) {
                        std::this_thread::yield();
                    }
                    // Other thread's writes before the release store are visible
                    // in this thread's read after the acquire load
                    assert(non_atomic == 6);
                }));
            }

            non_atomic = 6;
            store_op(at, old_value, new_value);

            for (auto& thread : threads) {
                thread.join();
            }
        }
    #else
        (void)store_op;
        (void)load_op;
    #endif
    }
}

TEST(fences)
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

TEST(flag)
{
    {
        std::atomic_flag f;
        f.clear();
        f.test_and_set();
        std::atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        f.test_and_set();
        std::atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f; // uninitialized first
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test(&f) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test(&f) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test(&f) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test(&f) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set(&f) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set(&f) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 1);
    }
    {
        std::atomic_flag f; // uninitialized
        f.clear();
        assert(f.test_and_set() == 0);
        f.clear();
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear(std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear(std::memory_order_release);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear(std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set() == 0);
        f.clear();
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear(std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear(std::memory_order_release);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear(std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        f.clear(std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set() == 0);
        {
            typedef std::atomic_flag A;
            TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {1};
            A& zero = *new (storage) A();
            assert(!zero.test_and_set());
            zero.~A();
        }
    }
    {
        std::atomic_flag f = ATOMIC_FLAG_INIT;
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set() == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_relaxed) == 0);
        assert(f.test_and_set(std::memory_order_relaxed) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_consume) == 0);
        assert(f.test_and_set(std::memory_order_consume) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_acquire) == 0);
        assert(f.test_and_set(std::memory_order_acquire) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_release) == 0);
        assert(f.test_and_set(std::memory_order_release) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_acq_rel) == 0);
        assert(f.test_and_set(std::memory_order_acq_rel) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_seq_cst) == 0);
        assert(f.test_and_set(std::memory_order_seq_cst) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set() == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_relaxed) == 0);
        assert(f.test_and_set(std::memory_order_relaxed) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_consume) == 0);
        assert(f.test_and_set(std::memory_order_consume) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_acquire) == 0);
        assert(f.test_and_set(std::memory_order_acquire) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_release) == 0);
        assert(f.test_and_set(std::memory_order_release) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_acq_rel) == 0);
        assert(f.test_and_set(std::memory_order_acq_rel) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(f.test_and_set(std::memory_order_seq_cst) == 0);
        assert(f.test_and_set(std::memory_order_seq_cst) == 1);
    }

}

TEST(general)
{
    std::atomic<int> i;
    volatile std::atomic<int> v;
    int exp = 0;

    (void)i.compare_exchange_weak(exp, 0, std::memory_order_acq_rel);
    (void)i.compare_exchange_weak(exp, 0, std::memory_order_release);
    i.compare_exchange_strong(exp, 0, std::memory_order_acq_rel);
    i.compare_exchange_strong(exp, 0, std::memory_order_release);

    (void)v.compare_exchange_weak(exp, 0, std::memory_order_acq_rel);
    (void)v.compare_exchange_weak(exp, 0, std::memory_order_release);
    v.compare_exchange_strong(exp, 0, std::memory_order_acq_rel);
    v.compare_exchange_strong(exp, 0, std::memory_order_release);
}

namespace
{
    template <typename T>
    void checkAlwaysLockFree()
    {
        if (std::atomic<T>::is_always_lock_free) {
            assert(std::atomic<T>().is_lock_free());
        }
    }
}

TEST(lockfree)
{
    // structs and unions can't be defined in the template invocation.
    // Work around this with a typedef.
#define CHECK_ALWAYS_LOCK_FREE(T)                                              \
  do {                                                                         \
    typedef T type;                                                            \
    checkAlwaysLockFree<type>();                                               \
  } while (0)

    CHECK_ALWAYS_LOCK_FREE(bool);
    CHECK_ALWAYS_LOCK_FREE(char);
    CHECK_ALWAYS_LOCK_FREE(signed char);
    CHECK_ALWAYS_LOCK_FREE(unsigned char);
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    CHECK_ALWAYS_LOCK_FREE(char8_t);
#endif
    CHECK_ALWAYS_LOCK_FREE(char16_t);
    CHECK_ALWAYS_LOCK_FREE(char32_t);
    CHECK_ALWAYS_LOCK_FREE(wchar_t);
    CHECK_ALWAYS_LOCK_FREE(short);
    CHECK_ALWAYS_LOCK_FREE(unsigned short);
    CHECK_ALWAYS_LOCK_FREE(int);
    CHECK_ALWAYS_LOCK_FREE(unsigned int);
    CHECK_ALWAYS_LOCK_FREE(long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long);
    CHECK_ALWAYS_LOCK_FREE(long long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long long);
    CHECK_ALWAYS_LOCK_FREE(std::nullptr_t);
    CHECK_ALWAYS_LOCK_FREE(void*);
    CHECK_ALWAYS_LOCK_FREE(float);
    CHECK_ALWAYS_LOCK_FREE(double);
    CHECK_ALWAYS_LOCK_FREE(long double);
    CHECK_ALWAYS_LOCK_FREE(struct Empty {});
    CHECK_ALWAYS_LOCK_FREE(struct OneInt {
        int i;
    });
    CHECK_ALWAYS_LOCK_FREE(struct IntArr2 {
        int i[2];
    });
    CHECK_ALWAYS_LOCK_FREE(struct FloatArr3 {
        float i[3];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr2 {
        long long int i[2];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr4 {
        long long int i[4];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr8 {
        long long int i[8];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr16 {
        long long int i[16];
    });
    CHECK_ALWAYS_LOCK_FREE(struct Padding {
        char c; /* padding */ long long int i;
    });
    CHECK_ALWAYS_LOCK_FREE(union IntFloat {
        int i; float f;
    });
    CHECK_ALWAYS_LOCK_FREE(enum class CharEnumClass : char {
        foo
    });

#undef CHECK_ALWAYS_LOCK_FREE

    // C macro and static constexpr must be consistent.
    enum class CharEnumClass : char {
        foo
    };
    static_assert(std::atomic<bool>::is_always_lock_free == (2 == ATOMIC_BOOL_LOCK_FREE), "");
    static_assert(std::atomic<char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<CharEnumClass>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<signed char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
    static_assert(std::atomic<unsigned char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE), "");
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    static_assert(std::atomic<char8_t>::is_always_lock_free == (2 == ATOMIC_CHAR8_T_LOCK_FREE), "");
#endif
    static_assert(std::atomic<char16_t>::is_always_lock_free == (2 == ATOMIC_CHAR16_T_LOCK_FREE), "");
    static_assert(std::atomic<char32_t>::is_always_lock_free == (2 == ATOMIC_CHAR32_T_LOCK_FREE), "");
    static_assert(std::atomic<wchar_t>::is_always_lock_free == (2 == ATOMIC_WCHAR_T_LOCK_FREE), "");
    static_assert(std::atomic<short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE), "");
    static_assert(std::atomic<unsigned short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE), "");
    static_assert(std::atomic<int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE), "");
    static_assert(std::atomic<unsigned int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE), "");
    static_assert(std::atomic<long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE), "");
    static_assert(std::atomic<unsigned long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE), "");
    static_assert(std::atomic<long long>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE), "");
    static_assert(std::atomic<unsigned long long>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE), "");
    static_assert(std::atomic<void*>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE), "");
    static_assert(std::atomic<std::nullptr_t>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE), "");

#if TEST_STD_VER >= 20
    static_assert(std::atomic_signed_lock_free::is_always_lock_free, "");
    static_assert(std::atomic_unsigned_lock_free::is_always_lock_free, "");
#endif

}

TEST(order)
{
    assert(std::kill_dependency(5) == 5);
    assert(std::kill_dependency(-5.5) == -5.5);

    assert(static_cast<int>(std::memory_order_relaxed) == 0);
    assert(static_cast<int>(std::memory_order_consume) == 1);
    assert(static_cast<int>(std::memory_order_acquire) == 2);
    assert(static_cast<int>(std::memory_order_release) == 3);
    assert(static_cast<int>(std::memory_order_acq_rel) == 4);
    assert(static_cast<int>(std::memory_order_seq_cst) == 5);

    std::memory_order o = std::memory_order_seq_cst;
    assert(static_cast<int>(o) == 5);

    static_assert(std::memory_order_relaxed == std::memory_order::relaxed);
    static_assert(std::memory_order_consume == std::memory_order::consume);
    static_assert(std::memory_order_acquire == std::memory_order::acquire);
    static_assert(std::memory_order_release == std::memory_order::release);
    static_assert(std::memory_order_acq_rel == std::memory_order::acq_rel);
    static_assert(std::memory_order_seq_cst == std::memory_order::seq_cst);
}

namespace
{
    template <typename T>
    struct TestAssign {
        void operator()() const
        {
            {
                T x(T(1));
                std::atomic_ref<T> const a(x);

                std::same_as<T> decltype(auto) y = (a = T(2));
                assert(y == T(2));
                assert(x == T(2));

                ASSERT_NOEXCEPT(a = T(0));
                static_assert(std::is_nothrow_assignable_v<std::atomic_ref<T>, T>);

                static_assert(!std::is_copy_assignable_v<std::atomic_ref<T>>);
            }

            {
                auto assign = [](std::atomic_ref<T> const& y, T, T new_val)
                {
                    y = new_val;
                };
                auto load = [](std::atomic_ref<T> const& y)
                {
                    return y.load();
                };
                test_seq_cst<T>(assign, load);
            }
        }
    };
}

template <typename T>
concept has_bitwise_and_assign = requires { std::declval<T const>() &= std::declval<T>(); };

template <typename T>
struct TestDoesNotHaveBitwiseAndAssign {
    void operator()() const
    {
        static_assert(!has_bitwise_and_assign<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestBitwiseAndAssign {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        std::same_as<T> decltype(auto) y = (a &= T(1));
        assert(y == T(1));
        assert(x == T(1));
        ASSERT_NOEXCEPT(a &= T(0));

        y = (a &= T(2));
        assert(y == T(0));
        assert(x == T(0));
    }
};

template <typename T>
concept has_bitwise_or_assign = requires { std::declval<T const>() |= std::declval<T>(); };

template < typename T>
struct TestDoesNotHaveBitwiseOrAssign {
    void operator()() const
    {
        static_assert(!has_bitwise_or_assign<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestBitwiseOrAssign {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        std::same_as<T> decltype(auto) y = (a |= T(2));
        assert(y == T(3));
        assert(x == T(3));
        ASSERT_NOEXCEPT(a |= T(0));
    }
};

template <typename T>
concept has_bitwise_xor_assign = requires { std::declval<T const>() ^= std::declval<T>(); };

template <typename T>
struct TestDoesNotHaveBitwiseXorAssign {
    void operator()() const
    {
        static_assert(!has_bitwise_xor_assign<std::atomic_ref<float>>);
    }
};

template <typename T>
struct TestBitwiseXorAssign {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        std::same_as<T> decltype(auto) y = (a ^= T(2));
        assert(y == T(3));
        assert(x == T(3));
        ASSERT_NOEXCEPT(a ^= T(0));
    }
};

template <typename T>
struct TestCompareExchangeStrong {
    void operator()() const
    {
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y = a.compare_exchange_strong(t, T(2));
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_strong(t, T(3));
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_strong(t, T(2)));
        }
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y = a.compare_exchange_strong(t, T(2), std::memory_order_seq_cst);
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_strong(t, T(3), std::memory_order_seq_cst);
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_strong(t, T(2), std::memory_order_seq_cst));
        }
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y =
                a.compare_exchange_strong(t, T(2), std::memory_order_release, std::memory_order_relaxed);
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_strong(t, T(3), std::memory_order_release, std::memory_order_relaxed);
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_strong(t, T(2), std::memory_order_release, std::memory_order_relaxed));
        }

        // success memory_order::release
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::release, std::memory_order::relaxed);
                assert(r);
            };

            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::release);
                assert(r);
            };
            test_acquire_release<T>(store_one_arg, load);
        }

        // success memory_order::acquire
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };

            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acquire, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load);

            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acquire)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load_one_arg);
        }

        // success memory_order::acq_rel
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::acq_rel, std::memory_order::relaxed);
                assert(r);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acq_rel, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::acq_rel);
                assert(r);
            };
            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acq_rel)) {
                }
                return val;
            };
            test_acquire_release<T>(store_one_arg, load_one_arg);
        }

        // success memory_order::seq_cst
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::seq_cst, std::memory_order::relaxed);
                assert(r);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::seq_cst, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_seq_cst<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::seq_cst);
                assert(r);
            };
            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::seq_cst)) {
                }
                return val;
            };
            test_seq_cst<T>(store_one_arg, load_one_arg);
        }

        // failure memory_order::acquire
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r =
                    x.compare_exchange_strong(unexpected, unexpected, std::memory_order::relaxed, std::memory_order::acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load);

            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r = x.compare_exchange_strong(unexpected, unexpected, std::memory_order::acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load_one_arg);

            // acq_rel replaced by acquire
            auto load_one_arg_acq_rel = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r = x.compare_exchange_strong(unexpected, unexpected, std::memory_order::acq_rel);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load_one_arg_acq_rel);
        }

        // failure memory_order::seq_cst
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r =
                    x.compare_exchange_strong(unexpected, unexpected, std::memory_order::relaxed, std::memory_order::seq_cst);
                assert(!r);
                return result;
            };
            test_seq_cst<T>(store, load);
        }
    }
};


template <typename T>
struct TestCompareExchangeWeak {
    void operator()() const
    {
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y = a.compare_exchange_weak(t, T(2));
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_weak(t, T(3));
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_weak(t, T(2)));
        }
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y = a.compare_exchange_weak(t, T(2), std::memory_order_seq_cst);
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_weak(t, T(3), std::memory_order_seq_cst);
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_weak(t, T(2), std::memory_order_seq_cst));
        }
        {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            T t(T(1));
            std::same_as<bool> decltype(auto) y =
                a.compare_exchange_weak(t, T(2), std::memory_order_release, std::memory_order_relaxed);
            assert(y == true);
            assert(a == T(2));
            assert(t == T(1));
            y = a.compare_exchange_weak(t, T(3), std::memory_order_release, std::memory_order_relaxed);
            assert(y == false);
            assert(a == T(2));
            assert(t == T(2));

            ASSERT_NOEXCEPT(a.compare_exchange_weak(t, T(2), std::memory_order_release, std::memory_order_relaxed));
        }

        // success memory_order::release
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::release, std::memory_order::relaxed)) {
                }
            };

            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::release)) {
                }
            };
            test_acquire_release<T>(store_one_arg, load);
        }

        // success memory_order::acquire
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acquire, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load);

            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acquire)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load_one_arg);
        }

        // success memory_order::acq_rel
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::acq_rel, std::memory_order::relaxed)) {
                }
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acq_rel, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_acquire_release<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::acq_rel)) {
                }
            };
            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acq_rel)) {
                }
                return val;
            };
            test_acquire_release<T>(store_one_arg, load_one_arg);
        }

        // success memory_order::seq_cst
        {
            auto store = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::seq_cst, std::memory_order::relaxed)) {
                }
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::seq_cst, std::memory_order::relaxed)) {
                }
                return val;
            };
            test_seq_cst<T>(store, load);

            auto store_one_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::seq_cst)) {
                }
            };
            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::seq_cst)) {
                }
                return val;
            };
            test_seq_cst<T>(store_one_arg, load_one_arg);
        }

        // failure memory_order::acquire
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r =
                    x.compare_exchange_weak(unexpected, unexpected, std::memory_order::relaxed, std::memory_order::acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load);

            auto load_one_arg = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order::acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load_one_arg);

            // acq_rel replaced by acquire
            auto load_one_arg_acq_rel = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order::acq_rel);
                assert(!r);
                return result;
            };
            test_acquire_release<T>(store, load_one_arg_acq_rel);
        }

        // failure memory_order::seq_cst
        {
            auto store = [](std::atomic_ref<T> const& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(255));
                bool r =
                    x.compare_exchange_weak(unexpected, unexpected, std::memory_order::relaxed, std::memory_order::seq_cst);
                assert(!r);
                return result;
            };
            test_seq_cst<T>(store, load);
        }
    }
};

template <typename T>
struct TestConvert {
    void operator()() const
    {
        T x(T(1));

        T copy = x;
        std::atomic_ref<T> const a(copy);

        T converted = a;
        assert(converted == x);

        ASSERT_NOEXCEPT(T(a));
        static_assert(std::is_nothrow_convertible_v<std::atomic_ref<T>, T>);

        auto store = [](std::atomic_ref<T> const& y, T, T new_val)
        {
            y.store(new_val);
        };
        auto load = [](std::atomic_ref<T> const& y)
        {
            return static_cast<T>(y);
        };
        test_seq_cst<T>(store, load);
    }
};

template <typename T>
struct TestCtor {
    void operator()() const
    {
        // check that the constructor is explicit
        static_assert(!std::is_convertible_v<T, std::atomic_ref<T>>);
        static_assert(std::is_constructible_v<std::atomic_ref<T>, T&>);

        T x(T(0));
        std::atomic_ref<T> a(x);
        (void)a;
    }
};

template <typename T>
struct TestDeduction {
    void operator()() const
    {
        T x(T(0));
        std::atomic_ref a(x);
        ASSERT_SAME_TYPE(decltype(a), std::atomic_ref<T>);
    }
};

template <typename T>
struct TestExchange {
    void operator()() const
    {
        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = a.exchange(T(2));
            assert(y == T(1));
            ASSERT_NOEXCEPT(a.exchange(T(2)));
        }

        {
            std::same_as<T> decltype(auto) y = a.exchange(T(3), std::memory_order_seq_cst);
            assert(y == T(2));
            ASSERT_NOEXCEPT(a.exchange(T(3), std::memory_order_seq_cst));
        }
    }
};

template <typename T>
concept has_fetch_add = requires {
    std::declval<T const>().fetch_add(std::declval<T>());
    std::declval<T const>().fetch_add(std::declval<T>(), std::declval<std::memory_order>());
};

template <typename T>
struct TestDoesNotHaveFetchAdd {
    void operator()() const
    {
        static_assert(!has_fetch_add<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestFetchAdd {
    void operator()() const
    {
        if constexpr (std::is_arithmetic_v<T>) {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            {
                std::same_as<T> decltype(auto) y = a.fetch_add(T(2));
                assert(y == T(1));
                assert(x == T(3));
                ASSERT_NOEXCEPT(a.fetch_add(T(0)));
            }

            {
                std::same_as<T> decltype(auto) y = a.fetch_add(T(4), std::memory_order_relaxed);
                assert(y == T(3));
                assert(x == T(7));
                ASSERT_NOEXCEPT(a.fetch_add(T(0), std::memory_order_relaxed));
            }
        }
        else if constexpr (std::is_pointer_v<T>) {
            using U = std::remove_pointer_t<T>;
            U t[9] = {};
            T p{&t[1]};
            std::atomic_ref<T> const a(p);

            {
                std::same_as<T> decltype(auto) y = a.fetch_add(2);
                assert(y == &t[1]);
                assert(a == &t[3]);
                ASSERT_NOEXCEPT(a.fetch_add(0));
            }

            {
                std::same_as<T> decltype(auto) y = a.fetch_add(4, std::memory_order_relaxed);
                assert(y == &t[3]);
                assert(a == &t[7]);
                ASSERT_NOEXCEPT(a.fetch_add(0, std::memory_order_relaxed));
            }
        }
        else {
            static_assert(std::is_void_v<T>);
        }

        // memory_order::release
        {
            auto fetch_add = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_add(new_val - old_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(fetch_add, load);
        }

        // memory_order::seq_cst
        {
            auto fetch_add_no_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_add(new_val - old_val);
            };
            auto fetch_add_with_order = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_add(new_val - old_val, std::memory_order::seq_cst);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load();
            };
            test_seq_cst<T>(fetch_add_no_arg, load);
            test_seq_cst<T>(fetch_add_with_order, load);
        }
    }
};

template <typename T>
concept has_fetch_and = requires {
    std::declval<T const>().fetch_and(std::declval<T>());
    std::declval<T const>().fetch_and(std::declval<T>(), std::declval<std::memory_order>());
};

template <typename T>
struct TestDoesNotHaveFetchAnd {
    void operator()() const
    {
        static_assert(!has_fetch_and<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestFetchAnd {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = a.fetch_and(T(2));
            assert(y == T(1));
            assert(x == T(0));
            ASSERT_NOEXCEPT(a.fetch_and(T(0)));
        }

        x = T(1);

        {
            std::same_as<T> decltype(auto) y = a.fetch_and(T(2), std::memory_order_relaxed);
            assert(y == T(1));
            assert(x == T(0));
            ASSERT_NOEXCEPT(a.fetch_and(T(0), std::memory_order_relaxed));
        }
    }
};

template <typename T>
concept has_fetch_or = requires {
    std::declval<T const>().fetch_or(std::declval<T>());
    std::declval<T const>().fetch_or(std::declval<T>(), std::declval<std::memory_order>());
};

template <typename T>
struct TestDoesNotHaveFetchOr {
    void operator()() const
    {
        static_assert(!has_fetch_or<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestFetchOr {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = a.fetch_or(T(2));
            assert(y == T(1));
            assert(x == T(3));
            ASSERT_NOEXCEPT(a.fetch_or(T(0)));
        }

        {
            std::same_as<T> decltype(auto) y = a.fetch_or(T(2), std::memory_order_relaxed);
            assert(y == T(3));
            assert(x == T(3));
            ASSERT_NOEXCEPT(a.fetch_or(T(0), std::memory_order_relaxed));
        }
    }
};

template <typename T>
concept has_fetch_sub = requires {
    std::declval<T const>().fetch_sub(std::declval<T>());
    std::declval<T const>().fetch_sub(std::declval<T>(), std::declval<std::memory_order>());
};

template <typename T>
struct TestDoesNotHaveFetchSub {
    void operator()() const
    {
        static_assert(!has_fetch_sub<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestFetchSub {
    void operator()() const
    {
        if constexpr (std::is_arithmetic_v<T>) {
            T x(T(7));
            std::atomic_ref<T> const a(x);

            {
                std::same_as<T> decltype(auto) y = a.fetch_sub(T(4));
                assert(y == T(7));
                assert(x == T(3));
                ASSERT_NOEXCEPT(a.fetch_sub(T(0)));
            }

            {
                std::same_as<T> decltype(auto) y = a.fetch_sub(T(2), std::memory_order_relaxed);
                assert(y == T(3));
                assert(x == T(1));
                ASSERT_NOEXCEPT(a.fetch_sub(T(0), std::memory_order_relaxed));
            }
        }
        else if constexpr (std::is_pointer_v<T>) {
            using U = std::remove_pointer_t<T>;
            U t[9] = {};
            T p{&t[7]};
            std::atomic_ref<T> const a(p);

            {
                std::same_as<T> decltype(auto) y = a.fetch_sub(4);
                assert(y == &t[7]);
                assert(a == &t[3]);
                ASSERT_NOEXCEPT(a.fetch_sub(0));
            }

            {
                std::same_as<T> decltype(auto) y = a.fetch_sub(2, std::memory_order_relaxed);
                assert(y == &t[3]);
                assert(a == &t[1]);
                ASSERT_NOEXCEPT(a.fetch_sub(0, std::memory_order_relaxed));
            }
        }
        else {
            static_assert(std::is_void_v<T>);
        }

        // memory_order::release
        {
            auto fetch_sub = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_sub(old_val - new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(fetch_sub, load);
        }

        // memory_order::seq_cst
        {
            auto fetch_sub_no_arg = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_sub(old_val - new_val);
            };
            auto fetch_sub_with_order = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x.fetch_sub(old_val - new_val, std::memory_order::seq_cst);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load();
            };
            test_seq_cst<T>(fetch_sub_no_arg, load);
            test_seq_cst<T>(fetch_sub_with_order, load);
        }
    }
};

template <typename T>
concept has_fetch_xor = requires {
    std::declval<T const>().fetch_xor(std::declval<T>());
    std::declval<T const>().fetch_xor(std::declval<T>(), std::declval<std::memory_order>());
};

template <typename T>
struct TestDoesNotHaveFetchXor {
    void operator()() const
    {
        static_assert(!has_fetch_xor<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestFetchXor {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = a.fetch_xor(T(2));
            assert(y == T(1));
            assert(x == T(3));
            ASSERT_NOEXCEPT(a.fetch_xor(T(0)));
        }

        {
            std::same_as<T> decltype(auto) y = a.fetch_xor(T(2), std::memory_order_relaxed);
            assert(y == T(3));
            assert(x == T(1));
            ASSERT_NOEXCEPT(a.fetch_xor(T(0), std::memory_order_relaxed));
        }
    }
};

template <typename T>
concept has_pre_increment_operator = requires { ++std::declval<T const>(); };

template <typename T>
concept has_post_increment_operator = requires { std::declval<T const>()++; };

template <typename T>
concept has_pre_decrement_operator = requires { --std::declval<T const>(); };

template <typename T>
concept has_post_decrement_operator = requires { std::declval<T const>()--; };

template <typename T>
constexpr bool does_not_have_increment_nor_decrement_operators()
{
    return !has_pre_increment_operator<T> && !has_pre_decrement_operator<T> && !has_post_increment_operator<T> &&
        !has_post_decrement_operator<T>;
}

template <typename T>
struct TestDoesNotHaveIncrementDecrement {
    void operator()() const
    {
        static_assert(does_not_have_increment_nor_decrement_operators<T>());
    }
};

template <typename T>
struct TestIncrementDecrement {
    void operator()() const
    {
        static_assert(std::is_integral_v<T>);

        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = ++a;
            assert(y == T(2));
            assert(x == T(2));
            ASSERT_NOEXCEPT(++a);
        }

        {
            std::same_as<T> decltype(auto) y = --a;
            assert(y == T(1));
            assert(x == T(1));
            ASSERT_NOEXCEPT(--a);
        }

        {
            std::same_as<T> decltype(auto) y = a++;
            assert(y == T(1));
            assert(x == T(2));
            ASSERT_NOEXCEPT(a++);
        }

        {
            std::same_as<T> decltype(auto) y = a--;
            assert(y == T(2));
            assert(x == T(1));
            ASSERT_NOEXCEPT(a--);
        }
    }
};

template <typename T>
struct TestLoad {
    void operator()() const
    {
        T x(T(1));
        std::atomic_ref<T> const a(x);

        {
            std::same_as<T> decltype(auto) y = a.load();
            assert(y == T(1));
            ASSERT_NOEXCEPT(a.load());
        }

        {
            std::same_as<T> decltype(auto) y = a.load(std::memory_order_seq_cst);
            assert(y == T(1));
            ASSERT_NOEXCEPT(a.load(std::memory_order_seq_cst));
        }

        // memory_order::seq_cst
        {
            auto store = [](std::atomic_ref<T> const& y, T, T new_val)
            {
                y.store(new_val);
            };
            auto load_no_arg = [](std::atomic_ref<T> const& y)
            {
                return y.load();
            };
            auto load_with_order = [](std::atomic_ref<T> const& y)
            {
                return y.load(std::memory_order::seq_cst);
            };
            test_seq_cst<T>(store, load_no_arg);
            test_seq_cst<T>(store, load_with_order);
        }

        // memory_order::release
        {
            auto store = [](std::atomic_ref<T> const& y, T, T new_val)
            {
                y.store(new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& y)
            {
                return y.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(store, load);
        }
    }
};

template <typename T>
concept has_operator_minus_equals = requires { std::declval<T const>() -= std::declval<T>(); };

template <typename T>
struct TestDoesNotHaveOperatorMinusEquals {
    void operator()() const
    {
        static_assert(!has_operator_minus_equals<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestOperatorMinusEquals {
    void operator()() const
    {
        if constexpr (std::is_arithmetic_v<T>) {
            T x(T(3));
            std::atomic_ref<T> const a(x);

            std::same_as<T> decltype(auto) y = (a -= T(2));
            assert(y == T(1));
            assert(x == T(1));
            ASSERT_NOEXCEPT(a -= T(0));
        }
        else if constexpr (std::is_pointer_v<T>) {
            using U = std::remove_pointer_t<T>;
            U t[9] = {};
            T p{&t[3]};
            std::atomic_ref<T> const a(p);

            std::same_as<T> decltype(auto) y = (a -= 2);
            assert(y == &t[1]);
            assert(a == &t[1]);
            ASSERT_NOEXCEPT(a -= 0);
        }
        else {
            static_assert(std::is_void_v<T>);
        }

        // memory_order::seq_cst
        {
            auto minus_equals = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x -= (old_val - new_val);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load();
            };
            test_seq_cst<T>(minus_equals, load);
        }
    }
};

template <typename T>
concept has_operator_plus_equals = requires { std::declval<T const>() += std::declval<T>(); };

template <typename T>
struct TestDoesNotHaveOperatorPlusEquals {
    void operator()() const
    {
        static_assert(!has_operator_plus_equals<std::atomic_ref<T>>);
    }
};

template <typename T>
struct TestOperatorPlusEquals {
    void operator()() const
    {
        if constexpr (std::is_arithmetic_v<T>) {
            T x(T(1));
            std::atomic_ref<T> const a(x);

            std::same_as<T> decltype(auto) y = (a += T(2));
            assert(y == T(3));
            assert(x == T(3));
            ASSERT_NOEXCEPT(a += T(0));
        }
        else if constexpr (std::is_pointer_v<T>) {
            using U = std::remove_pointer_t<T>;
            U t[9] = {};
            T p{&t[1]};
            std::atomic_ref<T> const a(p);

            std::same_as<T> decltype(auto) y = (a += 2);
            assert(y == &t[3]);
            assert(a == &t[3]);
            ASSERT_NOEXCEPT(a += 0);
        }
        else {
            static_assert(std::is_void_v<T>);
        }

        // memory_order::seq_cst
        {
            auto plus_equals = [](std::atomic_ref<T> const& x, T old_val, T new_val)
            {
                x += (new_val - old_val);
            };
            auto load = [](std::atomic_ref<T> const& x)
            {
                return x.load();
            };
            test_seq_cst<T>(plus_equals, load);
        }
    }
};

template <typename T>
struct TestStore {
    void operator()() const
    {
        T x(T(1));
        std::atomic_ref<T> const a(x);

        a.store(T(2));
        assert(x == T(2));
        ASSERT_NOEXCEPT(a.store(T(1)));

        a.store(T(3), std::memory_order_seq_cst);
        assert(x == T(3));
        ASSERT_NOEXCEPT(a.store(T(0), std::memory_order_seq_cst));

        // TODO memory_order::relaxed

        // memory_order::seq_cst
        {
            auto store_no_arg = [](std::atomic_ref<T> const& y, T, T new_val)
            {
                y.store(new_val);
            };
            auto store_with_order = [](std::atomic_ref<T> const& y, T, T new_val)
            {
                y.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](std::atomic_ref<T> const& y)
            {
                return y.load();
            };
            test_seq_cst<T>(store_no_arg, load);
            test_seq_cst<T>(store_with_order, load);
        }

        // memory_order::release
        {
            auto store = [](std::atomic_ref<T> const& y, T, T new_val)
            {
                y.store(new_val, std::memory_order::release);
            };
            auto load = [](std::atomic_ref<T> const& y)
            {
                return y.load(std::memory_order::acquire);
            };
            test_acquire_release<T>(store, load);
        }
    }
};

TEST(ref)
{
    TestEachAtomicType<TestAssign>()();


    TestEachIntegralType<TestBitwiseAndAssign>()();

    TestEachFloatingPointType<TestDoesNotHaveBitwiseAndAssign>()();

    TestEachPointerType<TestDoesNotHaveBitwiseAndAssign>()();

    TestDoesNotHaveBitwiseAndAssign<bool>()();
    TestDoesNotHaveBitwiseAndAssign<UserAtomicType>()();
    TestDoesNotHaveBitwiseAndAssign<LargeUserAtomicType>()();


    TestEachIntegralType<TestBitwiseOrAssign>()();

    TestEachFloatingPointType<TestDoesNotHaveBitwiseOrAssign>()();

    TestEachPointerType<TestDoesNotHaveBitwiseOrAssign>()();

    TestDoesNotHaveBitwiseOrAssign<bool>()();
    TestDoesNotHaveBitwiseOrAssign<UserAtomicType>()();
    TestDoesNotHaveBitwiseOrAssign<LargeUserAtomicType>()();


    TestEachIntegralType<TestBitwiseXorAssign>()();

    TestEachFloatingPointType<TestDoesNotHaveBitwiseXorAssign>()();

    TestEachPointerType<TestDoesNotHaveBitwiseXorAssign>()();

    TestDoesNotHaveBitwiseXorAssign<bool>()();
    TestDoesNotHaveBitwiseXorAssign<UserAtomicType>()();
    TestDoesNotHaveBitwiseXorAssign<LargeUserAtomicType>()();


    TestEachAtomicType<TestCompareExchangeStrong>()();

    TestEachAtomicType<TestCompareExchangeWeak>()();

    TestEachAtomicType<TestConvert>()();

    TestEachAtomicType<TestCtor>()();

    TestEachAtomicType<TestDeduction>()();

    TestEachAtomicType<TestExchange>()();

    TestEachIntegralType<TestFetchAdd>()();

    TestFetchAdd<float>()();
    TestFetchAdd<double>()();

    TestEachPointerType<TestFetchAdd>()();

    TestDoesNotHaveFetchAdd<bool>()();
    TestDoesNotHaveFetchAdd<UserAtomicType>()();
    TestDoesNotHaveFetchAdd<LargeUserAtomicType>()();

    TestEachIntegralType<TestFetchAnd>()();

    TestEachFloatingPointType<TestDoesNotHaveFetchAnd>()();

    TestEachPointerType<TestDoesNotHaveFetchAnd>()();

    TestDoesNotHaveFetchAnd<bool>()();
    TestDoesNotHaveFetchAnd<UserAtomicType>()();
    TestDoesNotHaveFetchAnd<LargeUserAtomicType>()();

    TestEachIntegralType<TestFetchOr>()();

    TestEachFloatingPointType<TestDoesNotHaveFetchOr>()();

    TestEachPointerType<TestDoesNotHaveFetchOr>()();

    TestDoesNotHaveFetchOr<bool>()();
    TestDoesNotHaveFetchOr<UserAtomicType>()();
    TestDoesNotHaveFetchOr<LargeUserAtomicType>()();

    TestEachIntegralType<TestFetchSub>()();

    TestFetchSub<float>()();
    TestFetchSub<double>()();

    TestEachPointerType<TestFetchSub>()();

    TestDoesNotHaveFetchSub<bool>()();
    TestDoesNotHaveFetchSub<UserAtomicType>()();
    TestDoesNotHaveFetchSub<LargeUserAtomicType>()();

    TestEachIntegralType<TestFetchXor>()();

    TestEachFloatingPointType<TestDoesNotHaveFetchXor>()();

    TestEachPointerType<TestDoesNotHaveFetchXor>()();

    TestDoesNotHaveFetchXor<bool>()();
    TestDoesNotHaveFetchXor<UserAtomicType>()();
    TestDoesNotHaveFetchXor<LargeUserAtomicType>()();

    TestEachIntegralType<TestIncrementDecrement>()();

    TestEachFloatingPointType<TestDoesNotHaveIncrementDecrement>()();

    TestEachPointerType<TestDoesNotHaveIncrementDecrement>()();

    TestDoesNotHaveIncrementDecrement<bool>()();
    TestDoesNotHaveIncrementDecrement<UserAtomicType>()();
    TestDoesNotHaveIncrementDecrement<LargeUserAtomicType>()();

    TestEachAtomicType<TestLoad>()();

    TestEachIntegralType<TestOperatorMinusEquals>()();

    TestOperatorMinusEquals<float>()();
    TestOperatorMinusEquals<double>()();

    TestEachPointerType<TestOperatorMinusEquals>()();

    TestDoesNotHaveOperatorMinusEquals<bool>()();
    TestDoesNotHaveOperatorMinusEquals<UserAtomicType>()();
    TestDoesNotHaveOperatorMinusEquals<LargeUserAtomicType>()();

    TestEachIntegralType<TestOperatorPlusEquals>()();

    TestOperatorPlusEquals<float>()();
    TestOperatorPlusEquals<double>()();

    TestEachPointerType<TestOperatorPlusEquals>()();

    TestDoesNotHaveOperatorPlusEquals<bool>()();
    TestDoesNotHaveOperatorPlusEquals<UserAtomicType>()();
    TestDoesNotHaveOperatorPlusEquals<LargeUserAtomicType>()();

    TestEachAtomicType<TestStore>()();
}

template <typename T>
void check_always_lock_free(std::atomic_ref<T> const a)
{
    std::same_as<const bool> decltype(auto) is_always_lock_free = std::atomic_ref<T>::is_always_lock_free;
    if (is_always_lock_free) {
        std::same_as<bool> decltype(auto) is_lock_free = a.is_lock_free();
        assert(is_lock_free);
    }
    ASSERT_NOEXCEPT(a.is_lock_free());
}

#define CHECK_ALWAYS_LOCK_FREE(T)                                                                                      \
  do {                                                                                                                 \
    typedef T type;                                                                                                    \
    type obj{};                                                                                                        \
    check_always_lock_free(std::atomic_ref<type>(obj));                                                                \
  } while (0)

TEST(test_is_always_lock_free)
{
    int i = 0;
    check_always_lock_free(std::atomic_ref<int>(i));

    float f = 0.f;
    check_always_lock_free(std::atomic_ref<float>(f));

    int* p = &i;
    check_always_lock_free(std::atomic_ref<int*>(p));

    CHECK_ALWAYS_LOCK_FREE(struct Empty {});
    CHECK_ALWAYS_LOCK_FREE(struct OneInt {
        int i;
    });
    CHECK_ALWAYS_LOCK_FREE(struct IntArr2 {
        int i[2];
    });
    CHECK_ALWAYS_LOCK_FREE(struct FloatArr3 {
        float i[3];
    });
    //CHECK_ALWAYS_LOCK_FREE(struct LLIArr2 {   // MSVC failed.
    //    long long int i[2];
    //});
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr4 {
        long long int i[4];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr8 {
        long long int i[8];
    });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr16 {
        long long int i[16];
    });
    //CHECK_ALWAYS_LOCK_FREE(struct Padding {   // MSVC failed.
    //    char c; /* padding */
    //    long long int i;
    //});
    CHECK_ALWAYS_LOCK_FREE(union IntFloat {
        int i;
        float f;
    });
    CHECK_ALWAYS_LOCK_FREE(enum class CharEnumClass : char {
        foo
    });

#undef  CHECK_ALWAYS_LOCK_FREE
}

template <class T>
concept has_difference_type = requires { typename T::difference_type; };

template <class T>
void check_member_types()
{
    if constexpr ((std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_floating_point_v<T>) {
        ASSERT_SAME_TYPE(typename std::atomic_ref<T>::value_type, T);
        ASSERT_SAME_TYPE(typename std::atomic_ref<T>::difference_type, T);
    }
    else if constexpr (std::is_pointer_v<T>) {
        ASSERT_SAME_TYPE(typename std::atomic_ref<T>::value_type, T);
        ASSERT_SAME_TYPE(typename std::atomic_ref<T>::difference_type, std::ptrdiff_t);
    }
    else {
        ASSERT_SAME_TYPE(typename std::atomic_ref<T>::value_type, T);
        static_assert(!has_difference_type<std::atomic_ref<T>>);
    }
}

template <class T>
void test_member_types()
{
    // value_type and difference_type (except for primary template)
    check_member_types<T>();

    static_assert(std::is_nothrow_copy_constructible_v<std::atomic_ref<T>>);

    static_assert(!std::is_copy_assignable_v<std::atomic_ref<T>>);

    // explicit constructor
    static_assert(!std::is_convertible_v<T, std::atomic_ref<T>>);
    static_assert(std::is_constructible_v<std::atomic_ref<T>, T&>);
}

TEST(member_types)
{
    // Primary template
    struct Empty {};
    test_member_types<Empty>();
    struct Trivial {
        int a;
        float b;
    };
    test_member_types<Trivial>();
    test_member_types<bool>();

    // Specialization for integral types
    // + character types
    test_member_types<char>();
    test_member_types<char8_t>();
    test_member_types<char16_t>();
    test_member_types<char32_t>();
    test_member_types<wchar_t>();
    // + standard signed integer types
    test_member_types<signed char>();
    test_member_types<short>();
    test_member_types<int>();
    test_member_types<long>();
    test_member_types<long long>();
    // + standard unsigned integer types
    test_member_types<unsigned char>();
    test_member_types<unsigned short>();
    test_member_types<unsigned int>();
    test_member_types<unsigned long>();
    test_member_types<unsigned long long>();
    // + any other types needed by the typedefs in the header <cstdint>
    test_member_types<int8_t>();
    test_member_types<int16_t>();
    test_member_types<int32_t>();
    test_member_types<int64_t>();
    test_member_types<int_fast8_t>();
    test_member_types<int_fast16_t>();
    test_member_types<int_fast32_t>();
    test_member_types<int_fast64_t>();
    test_member_types<int_least8_t>();
    test_member_types<int_least16_t>();
    test_member_types<int_least32_t>();
    test_member_types<int_least64_t>();
    test_member_types<intmax_t>();
    test_member_types<intptr_t>();
    test_member_types<uint8_t>();
    test_member_types<uint16_t>();
    test_member_types<uint32_t>();
    test_member_types<uint64_t>();
    test_member_types<uint_fast8_t>();
    test_member_types<uint_fast16_t>();
    test_member_types<uint_fast32_t>();
    test_member_types<uint_fast64_t>();
    test_member_types<uint_least8_t>();
    test_member_types<uint_least16_t>();
    test_member_types<uint_least32_t>();
    test_member_types<uint_least64_t>();
    test_member_types<uintmax_t>();
    test_member_types<uintptr_t>();

    // Specialization for floating-point types
    // + floating-point types
    test_member_types<float>();
    test_member_types<double>();
    test_member_types<long double>();
    // + TODO extended floating-point types
}
