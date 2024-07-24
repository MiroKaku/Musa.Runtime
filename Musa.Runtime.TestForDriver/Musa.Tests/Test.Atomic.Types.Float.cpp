#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"
#include "LLVM.TestSuite/make_test_thread.h"


namespace
{
    template <class T>
    bool approximately_equals(T x, T y)
    {
        T epsilon = T(0.001);
        return std::abs(x - y) < epsilon;
    }

    // Test that all threads see the exact same sequence of events
    // Test will pass 100% if store_op and load_op are correctly
    // affecting the memory with seq_cst order
    template <class T, template <class> class MaybeVolatile, class StoreOp, class LoadOp>
    void test_seq_cst(StoreOp store_op, LoadOp load_op)
    {
    #ifndef TEST_HAS_NO_THREADS
        for (int i = 0; i < 100; ++i) {
            T old_value = 0.0;
            T new_value = 1.0;

            MaybeVolatile<std::atomic<T>> x(old_value);
            MaybeVolatile<std::atomic<T>> y(old_value);

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
                while (!approximately_equals(load_op(x), new_value)) {
                    std::this_thread::yield();
                }
                if (!approximately_equals(load_op(y), new_value)) {
                    x_updated_first.store(true, std::memory_order_relaxed);
                }
            });

            auto t4 = support::make_test_thread([&]
            {
                while (!approximately_equals(load_op(y), new_value)) {
                    std::this_thread::yield();
                }
                if (!approximately_equals(load_op(x), new_value)) {
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
    template <class T, template <class> class MaybeVolatile, class StoreOp, class LoadOp>
    void test_acquire_release(StoreOp store_op, LoadOp load_op)
    {
    #ifndef TEST_HAS_NO_THREADS
        for (auto i = 0; i < 100; ++i) {
            T old_value = 0.0;
            T new_value = 1.0;

            MaybeVolatile<std::atomic<T>> at(old_value);
            int non_atomic = 5;

            constexpr auto number_of_threads = 8;
            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);

            for (auto j = 0; j < number_of_threads; ++j) {
                threads.push_back(support::make_test_thread([&at, &non_atomic, load_op, new_value]
                {
                    while (!approximately_equals(load_op(at), new_value)) {
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

namespace
{
    template<class T> concept HasVolatileAssign = requires(
        volatile std::atomic<T>& a,
        T                        t
    )
        {
            a = t;
        };

    template<class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_assign_impl()
    {
        static_assert(HasVolatileAssign<T> == std::atomic<T>::is_always_lock_free);

        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>() = (T(0))));

        // assignment
        {
            MaybeVolatile<std::atomic<T>>  a(T(3.1));
            std::same_as<T> decltype(auto) r = (a = T(1.2));
            assert(a.load() == T(1.2));
            assert(r == T(1.2));
        }

        // memory_order::seq_cst
        {
            auto assign = [](
                MaybeVolatile<std::atomic<T>>& x,
                T,
                T new_val
            )
            {
                x = new_val;
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(assign, load);
        }
    }

    template<class T>
    void test_assign()
    {
        test_assign_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_assign_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(assign)
{
    test_assign<float>();
    test_assign<double>();
}

namespace
{
    template<class T, class... Args> concept HasVolatileCompareExchangeStrong = requires(
        volatile std::atomic<T>& a,
        T                        t,
        Args...                  args
    )
    {
        a.compare_exchange_strong(t, t, args...);
    };

    template<class T, template <class> class MaybeVolatile, class... Args> concept HasNoexceptCompareExchangeStrong =
        requires(
        MaybeVolatile<std::atomic<T>>& a,
        T                              t,
        Args...                        args
    )
    {
        { a.compare_exchange_strong(t, t, args...) } noexcept;
    };

    template<class T, template <class> class MaybeVolatile = std::type_identity_t, class... MemoryOrder>
    void test_compare_exchange_strong_basic(
        MemoryOrder... memory_order
    )
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileCompareExchangeStrong<T, MemoryOrder...> == std::atomic<T>::is_always_lock_free);
        static_assert(HasNoexceptCompareExchangeStrong<T, MaybeVolatile, MemoryOrder...>);

        // compare pass
        {
            MaybeVolatile<std::atomic<T>>     a(T(1.2));
            T                                 expected(T(1.2));
            const T                           desired(T(2.3));
            std::same_as<bool> decltype(auto) r = a.compare_exchange_strong(expected, desired, memory_order...);

            assert(r);
            assert(a.load() == desired);
            assert(expected == T(1.2));
        }

        // compare fail
        {
            MaybeVolatile<std::atomic<T>>     a(T(1.2));
            T                                 expected(1.5);
            const T                           desired(T(2.3));
            std::same_as<bool> decltype(auto) r = a.compare_exchange_strong(expected, desired, memory_order...);

            assert(!r);
            assert(a.load() == T(1.2));
            assert(expected == T(1.2));
        }
    }

    template<class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_compare_exchange_strong_impl()
    {
        test_compare_exchange_strong_basic<T, MaybeVolatile>();
        test_compare_exchange_strong_basic<T, MaybeVolatile>(std::memory_order::relaxed);
        test_compare_exchange_strong_basic<T, MaybeVolatile>(std::memory_order::relaxed, std::memory_order_relaxed);

        // test success memory order release
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(
                    old_val,
                    new_val,
                    std::memory_order::release,
                    std::memory_order_relaxed);
                assert(r);
            };

            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::release);
                assert(r);
            };
            test_acquire_release<T, MaybeVolatile>(store_one_arg, load);
        }

        // test success memory order acquire
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T,
                T new_val
            )
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acquire, std::memory_order_relaxed)) {}
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto load_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acquire)) {}
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg);
        }

        // test success memory order acq_rel
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(
                    old_val,
                    new_val,
                    std::memory_order::acq_rel,
                    std::memory_order_relaxed);
                assert(r);
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acq_rel, std::memory_order_relaxed)) {}
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::acq_rel);
                assert(r);
            };
            auto load_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::acq_rel)) {}
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store_one_arg, load_one_arg);
        }

        // test success memory seq_cst
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(
                    old_val,
                    new_val,
                    std::memory_order::seq_cst,
                    std::memory_order_relaxed);
                assert(r);
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::seq_cst, std::memory_order_relaxed)) {}
                return val;
            };
            test_seq_cst<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x,
                T                              old_val,
                T                              new_val
            )
            {
                auto r = x.compare_exchange_strong(old_val, new_val, std::memory_order::seq_cst);
                assert(r);
            };
            auto load_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_strong(val, val, std::memory_order::seq_cst)) {}
                return val;
            };
            test_seq_cst<T, MaybeVolatile>(store_one_arg, load_one_arg);
        }

        // test fail memory order acquire
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T,
                T new_val
            )
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto result = x.load(std::memory_order::relaxed);
                T    unexpected(T(-9999.99));
                bool r = x.compare_exchange_strong(
                    unexpected,
                    unexpected,
                    std::memory_order_relaxed,
                    std::memory_order_acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto load_one_arg = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto result = x.load(std::memory_order::relaxed);
                T    unexpected(T(-9999.99));
                bool r = x.compare_exchange_strong(unexpected, unexpected, std::memory_order_acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg);

            // acq_rel replaced by acquire
            auto load_one_arg_acq_rel = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto result = x.load(std::memory_order::relaxed);
                T    unexpected(T(-9999.99));
                bool r = x.compare_exchange_strong(unexpected, unexpected, std::memory_order_acq_rel);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg_acq_rel);
        }

        // test fail memory order seq_cst
        {
            auto store = [](
                MaybeVolatile<std::atomic<T>>& x,
                T,
                T new_val
            )
            {
                x.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](
                MaybeVolatile<std::atomic<T>>& x
            )
            {
                auto result = x.load(std::memory_order::relaxed);
                T    unexpected(T(-9999.99));
                bool r = x.compare_exchange_strong(
                    unexpected,
                    unexpected,
                    std::memory_order_relaxed,
                    std::memory_order::seq_cst);
                assert(!r);
                return result;
            };
            test_seq_cst<T, MaybeVolatile>(store, load);
        }
    }

    template<class T>
    void test_compare_exchange_strong()
    {
        test_compare_exchange_strong_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_compare_exchange_strong_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(compare_exchange_strong)
{
    test_compare_exchange_strong<float>();
    test_compare_exchange_strong<double>();
}

namespace
{

    template <class T, class... Args>
    concept HasVolatileCompareExchangeWeak =
        requires(volatile std::atomic<T>&a, T t, Args... args)
    {
        a.compare_exchange_weak(t, t, args...);
    };

    template <class T, template <class> class MaybeVolatile, class... Args>
    concept HasNoexceptCompareExchangeWeak = requires(MaybeVolatile<std::atomic<T>>&a, T t, Args... args)
    {
        {
            a.compare_exchange_weak(t, t, args...)
        } noexcept;
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t, class... MemoryOrder>
    void test_compare_exchange_weak_basic(MemoryOrder... memory_order)
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileCompareExchangeWeak<T, MemoryOrder...> == std::atomic<T>::is_always_lock_free);
        static_assert(HasNoexceptCompareExchangeWeak<T, MaybeVolatile, MemoryOrder...>);

        // compare pass
        {
            MaybeVolatile<std::atomic<T>> a(T(1.2));
            T expected(T(1.2));
            const T desired(T(2.3));
            std::same_as<bool> decltype(auto) r = a.compare_exchange_weak(expected, desired, memory_order...);

            // could be false spuriously
            if (r) {
                assert(a.load() == desired);
            }
            // if r is true, expected should be unmodified (1.2)
            // if r is false, the original value of a (1.2) is written to expected
            assert(expected == T(1.2));
        }

        // compare fail
        {
            MaybeVolatile<std::atomic<T>> a(T(1.2));
            T expected(1.5);
            const T desired(T(2.3));
            std::same_as<bool> decltype(auto) r = a.compare_exchange_weak(expected, desired, memory_order...);

            assert(!r);
            assert(a.load() == T(1.2));

            // bug
            // TODO https://github.com/llvm/llvm-project/issues/47978
            if constexpr (!std::same_as<T, long double>) {
                assert(expected == T(1.2));
            }
        }
    }

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_compare_exchange_weak_impl()
    {
        test_compare_exchange_weak_basic<T, MaybeVolatile>();
        test_compare_exchange_weak_basic<T, MaybeVolatile>(std::memory_order::relaxed);
        test_compare_exchange_weak_basic<T, MaybeVolatile>(std::memory_order::relaxed, std::memory_order_relaxed);

        // test success memory order release
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::release, std::memory_order_relaxed)) {
                }
            };

            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::release)) {
                }
            };
            test_acquire_release<T, MaybeVolatile>(store_one_arg, load);
        }

        // test success memory order acquire
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acquire, std::memory_order_relaxed)) {
                }
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto load_one_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acquire)) {
                }
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg);
        }

        // test success memory order acq_rel
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::acq_rel, std::memory_order_relaxed)) {
                }
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acq_rel, std::memory_order_relaxed)) {
                }
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::acq_rel)) {
                }
            };
            auto load_one_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::acq_rel)) {
                }
                return val;
            };
            test_acquire_release<T, MaybeVolatile>(store_one_arg, load_one_arg);
        }

        // test success memory seq_cst
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::seq_cst, std::memory_order_relaxed)) {
                }
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::seq_cst, std::memory_order_relaxed)) {
                }
                return val;
            };
            test_seq_cst<T, MaybeVolatile>(store, load);

            auto store_one_arg = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                // could fail spuriously, so put it in a loop
                while (!x.compare_exchange_weak(old_val, new_val, std::memory_order::seq_cst)) {
                }
            };
            auto load_one_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto val = x.load(std::memory_order::relaxed);
                while (!x.compare_exchange_weak(val, val, std::memory_order::seq_cst)) {
                }
                return val;
            };
            test_seq_cst<T, MaybeVolatile>(store_one_arg, load_one_arg);
        }

        // test fail memory order acquire
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(-9999.99));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order_relaxed, std::memory_order_acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);

            auto load_one_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(-9999.99));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order_acquire);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg);

            // acq_rel replaced by acquire
            auto load_one_arg_acq_rel = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(-9999.99));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order_acq_rel);
                assert(!r);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load_one_arg_acq_rel);
        }

        // test fail memory order seq_cst
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                T unexpected(T(-9999.99));
                bool r = x.compare_exchange_weak(unexpected, unexpected, std::memory_order_relaxed, std::memory_order::seq_cst);
                assert(!r);
                return result;
            };
            test_seq_cst<T, MaybeVolatile>(store, load);
        }
    }

    template <class T>
    void test_compare_exchange_weak()
    {
        test_compare_exchange_weak_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_compare_exchange_weak_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(compare_exchange_weak)
{
    test_compare_exchange_weak<float>();
    test_compare_exchange_weak<double>();
}

namespace
{
    template <class T>
    constinit std::atomic<T> a1 = T();

    template <class T>
    constinit std::atomic<T> a2 = T(5.2);

    template <class T>
    constexpr void test_ctor_impl()
    {
        static_assert(std::is_nothrow_constructible_v<std::atomic<T>>);
        static_assert(std::is_nothrow_constructible_v<std::atomic<T>, T>);

        // constexpr atomic() noexcept;
        {
            std::atomic<T> a = {};
            if (!TEST_IS_CONSTANT_EVALUATED) {
                assert(a.load() == T(0));
            }
        }

        // constexpr atomic(floating-point-type) noexcept;
        {
            std::atomic<T> a = T(5.2);
            if (!TEST_IS_CONSTANT_EVALUATED) {
                assert(a.load() == T(5.2));
            }
        }

        // test constinit
        if (!TEST_IS_CONSTANT_EVALUATED) {
            assert(a1<T> == T(0.0));
            assert(a2<T> == T(5.2));
        }
    }

    constexpr bool test_ctor()
    {
        test_ctor_impl<float>();
        test_ctor_impl<double>();
        return true;
    }

    static_assert(test_ctor());

}

TEST(ctor)
{
    test_ctor();
}

namespace
{
    template <class T>
    concept HasVolatileExchange = requires(volatile std::atomic<T>&a, T t)
    {
        a.exchange(t);
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_exchange_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileExchange<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>() = (T(0))));

        // exchange
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            std::same_as<T> decltype(auto) r = a.exchange(T(1.2), std::memory_order::relaxed);
            assert(a.load() == T(1.2));
            assert(r == T(3.1));
        }

        // memory_order::release
        {
            auto exchange = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.exchange(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(exchange, load);
        }

        // memory_order::seq_cst
        {
            auto exchange_no_arg = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.exchange(new_val);
            };
            auto exchange_with_order = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.exchange(new_val, std::memory_order::seq_cst);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(exchange_no_arg, load);
            test_seq_cst<T, MaybeVolatile>(exchange_with_order, load);
        }
    }

    template <class T>
    void test_exchange()
    {
        test_exchange_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_exchange_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(exchange)
{
    test_exchange<float>();
    test_exchange<double>();
}

namespace
{
    template <class T>
    concept HasVolatileFetchAdd = requires(volatile std::atomic<T>&a, T t)
    {
        a.fetch_add(t);
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_fetch_add_impl()
    {
        static_assert(HasVolatileFetchAdd<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().fetch_add(T(0))));

        // fetch_add
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            std::same_as<T> decltype(auto) r = a.fetch_add(T(1.2), std::memory_order::relaxed);
            assert(r == T(3.1));
            assert(a.load() == T(3.1) + T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // fetch_add concurrent
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at;

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at.fetch_add(T(1.234), std::memory_order::relaxed);
                    }
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }

            const auto times = [](T t, int n)
            {
                T res(0);
                for (auto i = 0; i < n; ++i) {
                    res += t;
                }
                return res;
            };

            assert(at.load() == times(T(1.234), number_of_threads * loop));
        }
    #endif

        // memory_order::release
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                x.fetch_add(new_val - old_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);
        }

        // memory_order::seq_cst
        {
            auto fetch_add = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x.fetch_add(new_val - old_value);
            };
            auto fetch_add_with_order = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x.fetch_add(new_val - old_value, std::memory_order::seq_cst);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(fetch_add, load);
            test_seq_cst<T, MaybeVolatile>(fetch_add_with_order, load);
        }
    }

    template <class T>
    void test_fetch_add()
    {
        test_fetch_add_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_fetch_add_impl<T, std::add_volatile_t>();
        }
    }

}

TEST(fetch_add)
{
    test_fetch_add<float>();
    test_fetch_add<double>();
}

namespace
{
    template <class T>
    concept HasVolatileFetchSub = requires(volatile std::atomic<T>&a, T t)
    {
        a.fetch_sub(t);
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_fetch_sub_impl()
    {
        static_assert(HasVolatileFetchSub<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().fetch_sub(T(0))));

        // fetch_sub
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            std::same_as<T> decltype(auto) r = a.fetch_sub(T(1.2), std::memory_order::relaxed);
            assert(r == T(3.1));
            assert(a.load() == T(3.1) - T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // fetch_sub concurrent
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at;

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at.fetch_sub(T(1.234), std::memory_order::relaxed);
                    }
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }

            const auto accu_neg = [](T t, int n)
            {
                T res(0);
                for (auto i = 0; i < n; ++i) {
                    res -= t;
                }
                return res;
            };

            assert(at.load() == accu_neg(T(1.234), number_of_threads * loop));
        }
    #endif

        // memory_order::release
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T old_val, T new_val)
            {
                x.fetch_sub(old_val - new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);
        }

        // memory_order::seq_cst
        {
            auto fetch_sub = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x.fetch_sub(old_value - new_val);
            };
            auto fetch_sub_with_order = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x.fetch_sub(old_value - new_val, std::memory_order::seq_cst);
            };
            auto load = [](auto& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(fetch_sub, load);
            test_seq_cst<T, MaybeVolatile>(fetch_sub_with_order, load);
        }
    }

    template <class T>
    void test_fetch_sub()
    {
        test_fetch_sub_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_fetch_sub_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(fetch_sub)
{
    test_fetch_sub<float>();
    test_fetch_sub<double>();
}

namespace
{
    template <class T>
    concept HasVolatileLoad = requires(volatile std::atomic<T>&a, T t)
    {
        a.load();
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_load_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileLoad<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().load()));

        // load
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            a.store(T(1.2));
            std::same_as<T> decltype(auto) r = a.load(std::memory_order::relaxed);
            assert(r == T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // memory_order::relaxed
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at(T(-1.0));

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at, i]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at.store(T(i));
                    }
                }));
            }

            while (at.load(std::memory_order::relaxed) == T(-1.0)) {
                std::this_thread::yield();
            }

            for (auto i = 0; i < loop; ++i) {
                auto r = at.load(std::memory_order_relaxed);
                assert(std::ranges::any_of(std::views::iota(0, number_of_threads), [r](auto j)
                {
                    return r == T(j);
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }
        }

        // memory_order::consume
        {
            std::unique_ptr<T> p = std::make_unique<T>(T(0.0));
            MaybeVolatile<std::atomic<T>> at(T(0.0));

            constexpr auto number_of_threads = 8;
            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);

            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at, &p]
                {
                    while (at.load(std::memory_order::consume) == T(0.0)) {
                        std::this_thread::yield();
                    }
                    assert(*p == T(1.0)); // the write from other thread should be visible
                }));
            }

            *p = T(1.0);
            at.store(*p, std::memory_order_release);

            for (auto& thread : threads) {
                thread.join();
            }
        }
    #endif

        // memory_order::acquire
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);
        }

        // memory_order::seq_cst
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val);
            };
            auto load_no_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            auto load_with_order = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::seq_cst);
            };
            test_seq_cst<T, MaybeVolatile>(store, load_no_arg);
            test_seq_cst<T, MaybeVolatile>(store, load_with_order);
        }
    }

    template <class T>
    void test_load()
    {
        test_load_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_load_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(load)
{
    test_load<float>();
    test_load<double>();
}

namespace
{
    template <class T>
    concept isLockFreeNoexcept = requires(T t)
    {
        {
            t.is_lock_free()
        } noexcept;
    };

    template <class T>
    void test_lockfree()
    {
        static_assert(isLockFreeNoexcept<const std::atomic<T>&>);
        static_assert(isLockFreeNoexcept<const volatile std::atomic<T>&>);

        //   static constexpr bool is_always_lock_free = implementation-defined;
        {
            [[maybe_unused]] constexpr std::same_as<const bool> decltype(auto) r = std::atomic<T>::is_always_lock_free;
        }

        //   bool is_lock_free() const volatile noexcept;
        {
            const volatile std::atomic<T> a;
            std::same_as<bool> decltype(auto) r = a.is_lock_free();
            if (std::atomic<T>::is_always_lock_free) {
                assert(r);
            }
        }

        //   bool is_lock_free() const noexcept;
        {
            const std::atomic<T> a;
            std::same_as<bool> decltype(auto) r = a.is_lock_free();
            if (std::atomic<T>::is_always_lock_free) {
                assert(r);
            }
        }
    }
}

TEST(lockfree)
{
    test_lockfree<float>();
    test_lockfree<double>();
}

namespace
{
    template <class T>
    concept HasVolatileNotifyAll = requires(volatile std::atomic<T>&a, T t)
    {
        a.notify_all();
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_notify_all_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileNotifyAll<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().notify_all()));

        // bug?? wait can also fail for long double ??
        // should x87 80bit long double work at all?
        if constexpr (!std::same_as<T, long double>) {
            for (auto i = 0; i < 100; ++i) {
                const T old = T(3.1);
                MaybeVolatile<std::atomic<T>> a(old);

                bool done = false;
                std::atomic<int> started_num = 0;
                std::atomic<int> wait_done_num = 0;

                constexpr auto number_of_threads = 8;
                std::vector<std::thread> threads;
                threads.reserve(number_of_threads);

                for (auto j = 0; j < number_of_threads; ++j) {
                    threads.push_back(support::make_test_thread([&a, &started_num, old, &done, &wait_done_num]
                    {
                        started_num.fetch_add(1, std::memory_order::relaxed);

                        a.wait(old);
                        wait_done_num.fetch_add(1, std::memory_order::relaxed);

                        // likely to fail if wait did not block
                        assert(done);
                    }));
                }

                while (started_num.load(std::memory_order::relaxed) != number_of_threads) {
                    std::this_thread::yield();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                done = true;
                a.store(T(9.9));
                a.notify_all();

                // notify_all should unblock all the threads so that the loop below won't stuck
                while (wait_done_num.load(std::memory_order::relaxed) != number_of_threads) {
                    std::this_thread::yield();
                }

                for (auto& thread : threads) {
                    thread.join();
                }
            }
        }
    }

    template <class T>
    void test_notify_all()
    {
        test_notify_all_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_notify_all_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(notify_all)
{
    test_notify_all<float>();
    test_notify_all<double>();
}

namespace
{
    template <class T>
    concept HasVolatileNotifyOne = requires(volatile std::atomic<T>&a, T t)
    {
        a.notify_one();
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_notify_one_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileNotifyOne<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().notify_one()));

        // bug?? wait can also fail for long double ??
        // should x87 80bit long double work at all?
        if constexpr (!std::same_as<T, long double>) {
            for (auto i = 0; i < 100; ++i) {
                const T old = T(3.1);
                MaybeVolatile<std::atomic<T>> a(old);

                std::atomic_bool started = false;
                bool done = false;

                auto t = support::make_test_thread([&a, &started, old, &done]
                {
                    started.store(true, std::memory_order::relaxed);

                    a.wait(old);

                    // likely to fail if wait did not block
                    assert(done);
                });

                while (!started.load(std::memory_order::relaxed)) {
                    std::this_thread::yield();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                done = true;
                a.store(T(9.9));
                a.notify_one();
                t.join();
            }
        }
    }

    template <class T>
    void test_notify_one()
    {
        test_notify_one_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_notify_one_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(notify_one)
{
    test_notify_one<float>();
    test_notify_one<double>();
}

namespace
{
    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_operator_float_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(std::is_convertible_v<volatile std::atomic<T>&, T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(T(std::declval<MaybeVolatile<std::atomic<T>>&>())));

        // operator float
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            T r = a;
            assert(r == T(3.1));
        }

        // memory_order::seq_cst
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val);
            };
            auto op_float = [](MaybeVolatile<std::atomic<T>>& x) -> T
            {
                return x;
            };
            test_seq_cst<T, MaybeVolatile>(store, op_float);
        }
    }

    template <class T>
    void test_operator_float()
    {
        test_operator_float_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_operator_float_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(operator_float)
{
    test_operator_float<float>();
    test_operator_float<double>();
}

namespace
{
    template <class T>
    concept HasVolatileMinusEquals = requires(volatile std::atomic<T>&a, T t)
    {
        a -= t;
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_operator_minus_equals_impl()
    {
        static_assert(HasVolatileMinusEquals<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>() -= T(0)));

        // -=
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            std::same_as<T> decltype(auto) r = a -= T(1.2);
            assert(r == T(3.1) - T(1.2));
            assert(a.load() == T(3.1) - T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // -= concurrent
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at;

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at -= T(1.234);
                    }
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }

            const auto accu_neg = [](T t, int n)
            {
                T res(0);
                for (auto i = 0; i < n; ++i) {
                    res -= t;
                }
                return res;
            };

            assert(at.load() == accu_neg(T(1.234), number_of_threads * loop));
        }
    #endif

        // memory_order::seq_cst
        {
            auto minus_equals = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x -= (old_value - new_val);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(minus_equals, load);
        }
    }

    template <class T>
    void test_operator_minus_equals()
    {
        test_operator_minus_equals_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_operator_minus_equals_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(operator_minus_equals)
{
    test_operator_minus_equals<float>();
    test_operator_minus_equals<double>();
}

namespace
{
    template <class T>
    concept HasVolatilePlusEquals = requires(volatile std::atomic<T>&a, T t)
    {
        a += t;
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_operator_plus_equals_impl()
    {
        static_assert(HasVolatilePlusEquals<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>() += T(0)));

        // +=
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            std::same_as<T> decltype(auto) r = a += T(1.2);
            assert(r == T(3.1) + T(1.2));
            assert(a.load() == T(3.1) + T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // += concurrent
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at;

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at += T(1.234);
                    }
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }

            const auto times = [](T t, int n)
            {
                T res(0);
                for (auto i = 0; i < n; ++i) {
                    res += t;
                }
                return res;
            };

            assert(at.load() == times(T(1.234), number_of_threads * loop));
        }

        // memory_order::seq_cst
        {
            auto plus_equals = [](MaybeVolatile<std::atomic<T>>& x, T old_value, T new_val)
            {
                x += (new_val - old_value);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(plus_equals, load);
        }
    #endif
    }

    template <class T>
    void test_operator_plus_equals()
    {
        test_operator_plus_equals_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_operator_plus_equals_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(operator_plus_equals)
{
    test_operator_plus_equals<float>();
    test_operator_plus_equals<double>();
}

namespace
{
    template <class T>
    concept HasVolatileStore = requires(volatile std::atomic<T>&a, T t)
    {
        a.store(t);
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_store_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileStore<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().store(T(0))));

        // store
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            a.store(T(1.2), std::memory_order::relaxed);
            assert(a.load() == T(1.2));
        }

    #ifndef TEST_HAS_NO_THREADS
        // memory_order::relaxed
        {
            constexpr auto number_of_threads = 4;
            constexpr auto loop = 1000;

            MaybeVolatile<std::atomic<T>> at(T(-1.0));

            std::vector<std::thread> threads;
            threads.reserve(number_of_threads);
            for (auto i = 0; i < number_of_threads; ++i) {
                threads.push_back(support::make_test_thread([&at, i]()
                {
                    for (auto j = 0; j < loop; ++j) {
                        at.store(T(i), std::memory_order_relaxed);
                    }
                }));
            }

            while (at.load() == T(-1.0)) {
                std::this_thread::yield();
            }

            for (auto i = 0; i < loop; ++i) {
                auto r = at.load();
                assert(std::ranges::any_of(std::views::iota(0, number_of_threads), [r](auto j)
                {
                    return r == T(j);
                }));
            }

            for (auto& thread : threads) {
                thread.join();
            }
        }
    #endif

        // memory_order::release
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load(std::memory_order::acquire);
            };
            test_acquire_release<T, MaybeVolatile>(store, load);
        }

        // memory_order::seq_cst
        {
            auto store_no_arg = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val);
            };
            auto store_with_order = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::seq_cst);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                return x.load();
            };
            test_seq_cst<T, MaybeVolatile>(store_no_arg, load);
            test_seq_cst<T, MaybeVolatile>(store_with_order, load);
        }
    }

    template <class T>
    void test_store()
    {
        test_store_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_store_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(store)
{
    test_store<float>();
    test_store<double>();
}

namespace
{
    template <class T>
    concept HasVolatileWait = requires(volatile std::atomic<T>&a, T t)
    {
        a.wait(T());
    };

    template <class T, template <class> class MaybeVolatile = std::type_identity_t>
    void test_wait_impl()
    {
        // Uncomment the test after P1831R1 is implemented
        // static_assert(HasVolatileWait<T> == std::atomic<T>::is_always_lock_free);
        static_assert(noexcept(std::declval<MaybeVolatile<std::atomic<T>>&>().wait(T())));

        // wait with different value
        {
            MaybeVolatile<std::atomic<T>> a(T(3.1));
            a.wait(T(1.1), std::memory_order::relaxed);
        }

    #ifndef TEST_HAS_NO_THREADS
        // equal at the beginning and changed later
        // bug?? wait can also fail for long double ??
        // should x87 80bit long double work at all?
        if constexpr (!std::same_as<T, long double>) {
            for (auto i = 0; i < 100; ++i) {
                const T old = T(3.1);
                MaybeVolatile<std::atomic<T>> a(old);

                std::atomic_bool started = false;
                bool done = false;

                auto t = support::make_test_thread([&a, &started, old, &done]
                {
                    started.store(true, std::memory_order::relaxed);

                    a.wait(old);

                    // likely to fail if wait did not block
                    assert(done);
                });

                while (!started.load(std::memory_order::relaxed)) {
                    std::this_thread::yield();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                done = true;
                a.store(T(9.9));
                a.notify_all();
                t.join();
            }
        }
    #endif

        // memory_order::acquire
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val, std::memory_order::release);
            };
            auto load = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                x.wait(T(9999.999), std::memory_order::acquire);
                return result;
            };
            test_acquire_release<T, MaybeVolatile>(store, load);
        }

        // memory_order::seq_cst
        {
            auto store = [](MaybeVolatile<std::atomic<T>>& x, T, T new_val)
            {
                x.store(new_val);
            };
            auto load_no_arg = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                x.wait(T(9999.999));
                return result;
            };
            auto load_with_order = [](MaybeVolatile<std::atomic<T>>& x)
            {
                auto result = x.load(std::memory_order::relaxed);
                x.wait(T(9999.999), std::memory_order::seq_cst);
                return result;
            };
            test_seq_cst<T, MaybeVolatile>(store, load_no_arg);
            test_seq_cst<T, MaybeVolatile>(store, load_with_order);
        }
    }

    template <class T>
    void test_wait()
    {
        test_wait_impl<T>();
        if constexpr (std::atomic<T>::is_always_lock_free) {
            test_wait_impl<T, std::add_volatile_t>();
        }
    }
}

TEST(wait)
{
    test_wait<float>();
    test_wait<double>();
}
