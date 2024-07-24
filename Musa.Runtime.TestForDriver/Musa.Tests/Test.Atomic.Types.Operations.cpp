#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"
#include "LLVM.TestSuite/make_test_thread.h"
#include "LLVM.TestSuite/cmpxchg_loop.h"


#pragma warning(disable: 4996)

namespace
{
    template <class T>
    struct TestCompareExchangeStrong {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                T t(T(1));
                A a(t);
                assert(std::atomic_compare_exchange_strong(&a, &t, T(2)) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_strong(&a, &t, T(3)) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_strong(&a, &t, T(3)));
            }
            {
                typedef std::atomic<T> A;
                T t(T(1));
                volatile A a(t);
                assert(std::atomic_compare_exchange_strong(&a, &t, T(2)) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_strong(&a, &t, T(3)) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_strong(&a, &t, T(3)));
            }
        }
    };

}

TEST(atomic_compare_exchange_strong)
{
    TestEachAtomicType<TestCompareExchangeStrong>()();
}

namespace
{
    template <class T>
    struct TestCompareExchangeStrongExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                T t(T(1));
                A a(t);
                assert(std::atomic_compare_exchange_strong_explicit(&a, &t, T(2),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_strong_explicit(&a, &t, T(3),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_strong_explicit(&a, &t, T(3), std::memory_order_seq_cst,
                    std::memory_order_seq_cst));
            }
            {
                typedef std::atomic<T> A;
                T t(T(1));
                volatile A a(t);
                assert(std::atomic_compare_exchange_strong_explicit(&a, &t, T(2),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_strong_explicit(&a, &t, T(3),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_strong_explicit(&a, &t, T(3), std::memory_order_seq_cst,
                    std::memory_order_seq_cst));
            }
        }
    };
}

TEST(atomic_compare_exchange_strong_explicit)
{
    TestEachAtomicType<TestCompareExchangeStrongExplicit>()();
}

namespace
{
    template <class T>
    struct TestCompareExchangeWeak {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                T t(T(1));
                A a(t);
                assert(c_cmpxchg_weak_loop(&a, &t, T(2)) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_weak(&a, &t, T(3)) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_weak(&a, &t, T(3)));
            }
            {
                typedef std::atomic<T> A;
                T t(T(1));
                volatile A a(t);
                assert(c_cmpxchg_weak_loop(&a, &t, T(2)) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_weak(&a, &t, T(3)) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_weak(&a, &t, T(3)));
            }
        }
    };
}

TEST(atomic_compare_exchange_weak)
{
    TestEachAtomicType<TestCompareExchangeWeak>()();
}

namespace
{
    template <class T>
    struct TestCompareExchangeWeakExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                T t(T(1));
                A a(t);
                assert(c_cmpxchg_weak_loop(&a, &t, T(2),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_weak_explicit(&a, &t, T(3),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_weak_explicit(&a, &t, T(3), std::memory_order_seq_cst,
                    std::memory_order_seq_cst));
            }
            {
                typedef std::atomic<T> A;
                T t(T(1));
                volatile A a(t);
                assert(c_cmpxchg_weak_loop(&a, &t, T(2),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == true);
                assert(a == T(2));
                assert(t == T(1));
                assert(std::atomic_compare_exchange_weak_explicit(&a, &t, T(3),
                    std::memory_order_seq_cst, std::memory_order_seq_cst) == false);
                assert(a == T(2));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_compare_exchange_weak_explicit(&a, &t, T(3), std::memory_order_seq_cst,
                    std::memory_order_seq_cst));
            }
        }
    };
}

TEST(atomic_compare_exchange_weak_explicit)
{
    TestEachAtomicType<TestCompareExchangeWeakExplicit>()();
}

namespace
{
    template <class T>
    struct TestExchange{
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t(T(1));
            assert(std::atomic_exchange(&t, T(2)) == T(1));
            assert(t == T(2));
            volatile A vt(T(3));
            assert(std::atomic_exchange(&vt, T(4)) == T(3));
            assert(vt == T(4));

            ASSERT_NOEXCEPT(std::atomic_exchange(&t, T(2)));
            ASSERT_NOEXCEPT(std::atomic_exchange(&vt, T(4)));
        }
    };
}

TEST(atomic_exchange)
{
    TestEachAtomicType<TestExchange>()();
}

namespace
{
    template <class T>
    struct TestExchangeExplicit {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t(T(1));
            assert(std::atomic_exchange_explicit(&t, T(2), std::memory_order_seq_cst)
                == T(1));
            assert(t == T(2));
            volatile A vt(T(3));
            assert(std::atomic_exchange_explicit(&vt, T(4), std::memory_order_seq_cst)
                == T(3));
            assert(vt == T(4));

            ASSERT_NOEXCEPT(std::atomic_exchange_explicit(&t, T(2), std::memory_order_seq_cst));
            ASSERT_NOEXCEPT(std::atomic_exchange_explicit(&vt, T(4), std::memory_order_seq_cst));
        }
    };
}

TEST(atomic_exchange_explicit)
{
    TestEachAtomicType<TestExchangeExplicit>()();
}

namespace
{
    template <class T>
    struct TestFetchAdd {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_add(&t, T(2)) == T(1));
                assert(t == T(3));
                ASSERT_NOEXCEPT(std::atomic_fetch_add(&t, 0));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(1));
                assert(std::atomic_fetch_add(&t, T(2)) == T(1));
                assert(t == T(3));
                ASSERT_NOEXCEPT(std::atomic_fetch_add(&t, 0));
            }
        }
    };

    template <class T>
    void test_fetch_add()
    {
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            A t(&a[0]);
            assert(std::atomic_fetch_add(&t, 2) == &a[0]);
            std::atomic_fetch_add<T>(&t, 0);
            assert(t == &a[2]);
            ASSERT_NOEXCEPT(std::atomic_fetch_add(&t, 0));
        }
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            volatile A t(&a[0]);
            assert(std::atomic_fetch_add(&t, 2) == &a[0]);
            std::atomic_fetch_add<T>(&t, 0);
            assert(t == &a[2]);
            ASSERT_NOEXCEPT(std::atomic_fetch_add(&t, 0));
        }
    }

}

TEST(atomic_fetch_add)
{
    TestEachIntegralType<TestFetchAdd>()();

    test_fetch_add<int*>();
    test_fetch_add<const int*>();
}

namespace
{
    template <class T>
    struct TestFetchAddExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_add_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(1));
                assert(t == T(3));
                ASSERT_NOEXCEPT(std::atomic_fetch_add_explicit(&t, 0, std::memory_order_relaxed));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(1));
                assert(std::atomic_fetch_add_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(1));
                assert(t == T(3));
                ASSERT_NOEXCEPT(std::atomic_fetch_add_explicit(&t, 0, std::memory_order_relaxed));
            }
        }
    };

    template <class T>
    void test_fetch_add_explicit()
    {
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            A t(&a[0]);
            assert(std::atomic_fetch_add_explicit(&t, 2, std::memory_order_seq_cst) == &a[0]);
            std::atomic_fetch_add_explicit<T>(&t, 0, std::memory_order_relaxed);
            assert(t == &a[2]);
            ASSERT_NOEXCEPT(std::atomic_fetch_add_explicit(&t, 0, std::memory_order_relaxed));
        }
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            volatile A t(&a[0]);
            assert(std::atomic_fetch_add_explicit(&t, 2, std::memory_order_seq_cst) == &a[0]);
            std::atomic_fetch_add_explicit<T>(&t, 0, std::memory_order_relaxed);
            assert(t == &a[2]);
            ASSERT_NOEXCEPT(std::atomic_fetch_add_explicit(&t, 0, std::memory_order_relaxed));
        }
    }
}

TEST(atomic_fetch_add_explicit)
{
    TestEachIntegralType<TestFetchAddExplicit>()();

    test_fetch_add_explicit<int*>();
    test_fetch_add_explicit<const int*>();
}

namespace
{
    template <class T>
    struct TestFetchAnd {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_and(&t, T(2)) == T(1));
                assert(t == T(0));

                ASSERT_NOEXCEPT(std::atomic_fetch_and(&t, T(2)));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_and(&t, T(2)) == T(3));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_fetch_and(&t, T(2)));
            }
        }
    };
}

TEST(atomic_fetch_and)
{
    TestEachIntegralType<TestFetchAnd>()();
}

namespace
{
    template <class T>
    struct TestFetchAndExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_and_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(1));
                assert(t == T(0));

                ASSERT_NOEXCEPT(std::atomic_fetch_and_explicit(&t, T(2), std::memory_order_seq_cst));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_and_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(3));
                assert(t == T(2));

                ASSERT_NOEXCEPT(std::atomic_fetch_and_explicit(&t, T(2), std::memory_order_seq_cst));
            }
        }
    };
}

TEST(atomic_fetch_and_explicit)
{
    TestEachIntegralType<TestFetchAndExplicit>()();
}

namespace
{
    template <class T>
    struct TestFetchOr {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_or(&t, T(2)) == T(1));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_or(&t, T(2)));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_or(&t, T(2)) == T(3));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_or(&t, T(2)));
            }
        }
    };
}

TEST(atomic_fetch_or)
{
    TestEachIntegralType<TestFetchOr>()();
}

namespace
{
    template <class T>
    struct TestFetchOrExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_or_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(1));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_or_explicit(&t, T(2), std::memory_order_seq_cst));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_or_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(3));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_or_explicit(&t, T(2), std::memory_order_seq_cst));
            }
        }
    };
}

TEST(atomic_fetch_or_explicit)
{
    TestEachIntegralType<TestFetchOrExplicit>()();
}

namespace
{
    template <class T>
    struct TestFetchSub {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(3));
                assert(std::atomic_fetch_sub(&t, T(2)) == T(3));
                assert(t == T(1));
                ASSERT_NOEXCEPT(std::atomic_fetch_sub(&t, 0));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_sub(&t, T(2)) == T(3));
                assert(t == T(1));
                ASSERT_NOEXCEPT(std::atomic_fetch_sub(&t, 0));
            }
        }
    };

    template <class T>
    void test_fetch_sub()
    {
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            A t(&a[2]);
            assert(std::atomic_fetch_sub(&t, 2) == &a[2]);
            std::atomic_fetch_sub<T>(&t, 0);
            assert(t == &a[0]);
            ASSERT_NOEXCEPT(std::atomic_fetch_sub(&t, 0));
        }
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            volatile A t(&a[2]);
            assert(std::atomic_fetch_sub(&t, 2) == &a[2]);
            std::atomic_fetch_sub<T>(&t, 0);
            assert(t == &a[0]);
            ASSERT_NOEXCEPT(std::atomic_fetch_sub(&t, 0));
        }
    }

}

TEST(atomic_fetch_sub)
{
    TestEachIntegralType<TestFetchSub>()();

    test_fetch_sub<int*>();
    test_fetch_sub<const int*>();
}

namespace
{
    template <class T>
    struct TestFetchSubExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(3));
                assert(std::atomic_fetch_sub_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(3));
                assert(t == T(1));
                ASSERT_NOEXCEPT(std::atomic_fetch_sub_explicit(&t, 0, std::memory_order_relaxed));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_sub_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(3));
                assert(t == T(1));
                ASSERT_NOEXCEPT(std::atomic_fetch_sub_explicit(&t, 0, std::memory_order_relaxed));
            }
        }
    };

    template <class T>
    void test_fetch_sub_explicit()
    {
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            A t(&a[2]);
            assert(std::atomic_fetch_sub_explicit(&t, 2, std::memory_order_seq_cst) == &a[2]);
            std::atomic_fetch_sub_explicit<T>(&t, 0, std::memory_order_relaxed);
            assert(t == &a[0]);
            ASSERT_NOEXCEPT(std::atomic_fetch_sub_explicit(&t, 0, std::memory_order_relaxed));
        }
        {
            typedef std::atomic<T> A;
            typedef typename std::remove_pointer<T>::type X;
            X a[3] = {0};
            volatile A t(&a[2]);
            assert(std::atomic_fetch_sub_explicit(&t, 2, std::memory_order_seq_cst) == &a[2]);
            std::atomic_fetch_sub_explicit<T>(&t, 0, std::memory_order_relaxed);
            assert(t == &a[0]);
            ASSERT_NOEXCEPT(std::atomic_fetch_sub_explicit(&t, 0, std::memory_order_relaxed));
        }
    }
}

TEST(atomic_fetch_sub_explicit)
{
    TestEachIntegralType<TestFetchSubExplicit>()();

    test_fetch_sub_explicit<int*>();
    test_fetch_sub_explicit<const int*>();
}

namespace
{
    template <class T>
    struct TestTestFetchOr {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_xor(&t, T(2)) == T(1));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_xor(&t, T(2)));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_xor(&t, T(2)) == T(3));
                assert(t == T(1));

                ASSERT_NOEXCEPT(std::atomic_fetch_xor(&t, T(2)));
            }
        }
    };
}

TEST(atomic_fetch_xor)
{
    TestEachIntegralType<TestTestFetchOr>()();
}

namespace
{
    template <class T>
    struct TestTestFetchOrExplicit {
        void operator()() const
        {
            {
                typedef std::atomic<T> A;
                A t(T(1));
                assert(std::atomic_fetch_xor_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(1));
                assert(t == T(3));

                ASSERT_NOEXCEPT(std::atomic_fetch_xor_explicit(&t, T(2), std::memory_order_seq_cst));
            }
            {
                typedef std::atomic<T> A;
                volatile A t(T(3));
                assert(std::atomic_fetch_xor_explicit(&t, T(2),
                    std::memory_order_seq_cst) == T(3));
                assert(t == T(1));

                ASSERT_NOEXCEPT(std::atomic_fetch_xor_explicit(&t, T(2), std::memory_order_seq_cst));
            }
        }
    };
}

TEST(atomic_fetch_xor_explicit)
{
    TestEachIntegralType<TestTestFetchOrExplicit>()();
}

namespace
{
    template <class T>
    struct TestInit {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t;
            std::atomic_init(&t, T(1));
            assert(t == T(1));
            volatile A vt;
            std::atomic_init(&vt, T(2));
            assert(vt == T(2));

            ASSERT_NOEXCEPT(std::atomic_init(&t, T(1)));
            ASSERT_NOEXCEPT(std::atomic_init(&vt, T(2)));
        }
    };
}

TEST(atomic_init)
{
    TestEachAtomicType<TestInit>()();
}

namespace
{
    template <class T>
    struct TestIsLockFree {
        void operator()() const
        {
            typedef std::atomic<T> A;
            T t = T();

            A a(t);
            bool b1 = std::atomic_is_lock_free(static_cast<const A*>(&a));
        #if TEST_STD_VER >= 17
            if (A::is_always_lock_free)
                assert(b1);
        #endif

            volatile A va(t);
            bool b2 = std::atomic_is_lock_free(static_cast<const volatile A*>(&va));
            assert(b1 == b2);

            ASSERT_NOEXCEPT(std::atomic_is_lock_free(static_cast<const A*>(&a)));
            ASSERT_NOEXCEPT(std::atomic_is_lock_free(static_cast<const volatile A*>(&va)));
        }
    };

    struct A {
        char x[4];
    };
}

TEST(atomic_is_lock_free)
{
    TestIsLockFree<A>()();
    TestEachAtomicType<TestIsLockFree>()();
}

namespace
{
    template <class T>
    struct TestLoad {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t(T(1));
            assert(std::atomic_load(&t) == T(1));
            volatile A vt(T(2));
            assert(std::atomic_load(&vt) == T(2));

            ASSERT_NOEXCEPT(std::atomic_load(&t));
            ASSERT_NOEXCEPT(std::atomic_load(&vt));
        }
    };
}

TEST(atomic_load)
{
    TestEachAtomicType<TestLoad>()();
}

namespace
{
    template <class T>
    struct TestLoadExplicit {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t(T(1));
            assert(std::atomic_load_explicit(&t, std::memory_order_seq_cst) == T(1));
            volatile A vt(T(2));
            assert(std::atomic_load_explicit(&vt, std::memory_order_seq_cst) == T(2));

            ASSERT_NOEXCEPT(std::atomic_load_explicit(&t, std::memory_order_seq_cst));
            ASSERT_NOEXCEPT(std::atomic_load_explicit(&vt, std::memory_order_seq_cst));
        }
    };
}

TEST(atomic_load_explicit)
{
    TestEachAtomicType<TestLoadExplicit>()();
}

namespace
{
    template <class T>
    struct TestStore {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t;
            std::atomic_store(&t, T(1));
            assert(t == T(1));
            volatile A vt;
            std::atomic_store(&vt, T(2));
            assert(vt == T(2));

            ASSERT_NOEXCEPT(std::atomic_store(&t, T(1)));
            ASSERT_NOEXCEPT(std::atomic_store(&vt, T(2)));
        }
    };

}

TEST(atomic_store)
{
    TestEachAtomicType<TestStore>()();
}

namespace
{
    template <class T>
    struct TestStoreExplicit {
        void operator()() const
        {
            typedef std::atomic<T> A;
            A t;
            std::atomic_store_explicit(&t, T(1), std::memory_order_seq_cst);
            assert(t == T(1));
            volatile A vt;
            std::atomic_store_explicit(&vt, T(2), std::memory_order_seq_cst);
            assert(vt == T(2));

            ASSERT_NOEXCEPT(std::atomic_store_explicit(&t, T(1), std::memory_order_seq_cst));
            ASSERT_NOEXCEPT(std::atomic_store_explicit(&vt, T(2), std::memory_order_seq_cst));
        }
    };
}

TEST(atomic_store_explicit)
{
    TestEachAtomicType<TestStoreExplicit>()();
}

namespace
{
    struct UserType {
        int i;

        UserType() noexcept
        {}
        constexpr explicit UserType(int d) noexcept : i(d)
        {}

        friend bool operator==(const UserType& x, const UserType& y)
        {
            return x.i == y.i;
        }
    };

    template <class Tp>
    struct TestCtor {
        void operator()() const
        {
            typedef std::atomic<Tp> Atomic;
            constexpr Tp t(42);
            {
                constexpr Atomic a(t);
                assert(a == t);
            }
            {
                constexpr Atomic a{t};
                assert(a == t);
            }
        }
    };
}

TEST(ctor)
{
    TestCtor<UserType>()();
    TestEachIntegralType<TestCtor>()();
}

namespace
{
    template <class Tp>
    struct CheckTriviallyDestructible {
        void operator()() const
        {
            typedef std::atomic<Tp> Atomic;
            static_assert(std::is_trivially_destructible<Atomic>::value, "");
        }
    };
}

TEST(dtor)
{
    TestEachIntegralType<CheckTriviallyDestructible>()();
    TestEachFloatingPointType<CheckTriviallyDestructible>()();
    TestEachPointerType<CheckTriviallyDestructible>()();
}

namespace
{
    template <class T>
    struct TestNotifyAll {
        void operator()() const
        {
            typedef std::atomic<T> A;

            {
                A a(T(1));
                static_assert(noexcept(std::atomic_notify_all(&a)), "");

                std::atomic<bool> is_ready[2];
                is_ready[0] = false;
                is_ready[1] = false;
                auto f = [&](int index)
                {
                    assert(std::atomic_load(&a) == T(1));
                    is_ready[index].store(true);

                    std::atomic_wait(&a, T(1));
                    assert(std::atomic_load(&a) == T(3));
                };
                std::thread t1 = support::make_test_thread(f, /*index=*/0);
                std::thread t2 = support::make_test_thread(f, /*index=*/1);

                while (!is_ready[0] || !is_ready[1]) {
                    // Spin
                }
                std::atomic_store(&a, T(3));
                std::atomic_notify_all(&a);
                t1.join();
                t2.join();
            }
            {
                volatile A a(T(2));
                static_assert(noexcept(std::atomic_notify_all(&a)), "");

                std::atomic<bool> is_ready[2];
                is_ready[0] = false;
                is_ready[1] = false;
                auto f = [&](int index)
                {
                    assert(std::atomic_load(&a) == T(2));
                    is_ready[index].store(true);

                    std::atomic_wait(&a, T(2));
                    assert(std::atomic_load(&a) == T(4));
                };
                std::thread t1 = support::make_test_thread(f, /*index=*/0);
                std::thread t2 = support::make_test_thread(f, /*index=*/1);

                while (!is_ready[0] || !is_ready[1]) {
                    // Spin
                }
                std::atomic_store(&a, T(4));
                std::atomic_notify_all(&a);
                t1.join();
                t2.join();
            }
        }
    };
}

TEST(atomic_notify_all)
{
    TestEachAtomicType<TestNotifyAll>()();
}

namespace
{
    template <class T>
    struct TestNotifyOne {
        void operator()() const
        {
            typedef std::atomic<T> A;

            {
                A a(T(1));
                static_assert(noexcept(std::atomic_notify_one(&a)), "");
                std::thread t = support::make_test_thread([&]()
                {
                    std::atomic_store(&a, T(3));
                    std::atomic_notify_one(&a);
                });
                std::atomic_wait(&a, T(1));
                assert(std::atomic_load(&a) == T(3));
                t.join();
            }
            {
                volatile A a(T(2));
                static_assert(noexcept(std::atomic_notify_one(&a)), "");
                std::thread t = support::make_test_thread([&]()
                {
                    std::atomic_store(&a, T(4));
                    std::atomic_notify_one(&a);
                });
                std::atomic_wait(&a, T(2));
                assert(std::atomic_load(&a) == T(4));
                t.join();
            }
        }
    };
}

TEST(atomic_notify_one)
{
    TestEachAtomicType<TestNotifyOne>()();
}

namespace
{
    template <class T>
    struct TestWait {
        void operator()() const
        {
            typedef std::atomic<T> A;
            {
                A t(T(1));
                static_assert(noexcept(std::atomic_wait(&t, T(0))), "");
                assert(std::atomic_load(&t) == T(1));
                std::atomic_wait(&t, T(0));
                std::thread t1 = support::make_test_thread([&]()
                {
                    std::atomic_store(&t, T(3));
                    std::atomic_notify_one(&t);
                });
                std::atomic_wait(&t, T(1));
                assert(std::atomic_load(&t) == T(3));
                t1.join();
            }
            {
                volatile A vt(T(2));
                static_assert(noexcept(std::atomic_wait(&vt, T(0))), "");
                assert(std::atomic_load(&vt) == T(2));
                std::atomic_wait(&vt, T(1));
                std::thread t2 = support::make_test_thread([&]()
                {
                    std::atomic_store(&vt, T(4));
                    std::atomic_notify_one(&vt);
                });
                std::atomic_wait(&vt, T(2));
                assert(std::atomic_load(&vt) == T(4));
                t2.join();
            }
        }
    };
}

TEST(atomic_wait)
{
    TestEachAtomicType<TestWait>()();
}

namespace
{
    template <class T>
    struct TestWaitExplicit {
        void operator()() const
        {
            typedef std::atomic<T> A;
            {
                A t(T(1));
                static_assert(noexcept(std::atomic_wait_explicit(&t, T(0), std::memory_order_seq_cst)), "");
                assert(std::atomic_load(&t) == T(1));
                std::atomic_wait_explicit(&t, T(0), std::memory_order_seq_cst);
                std::thread t1 = support::make_test_thread([&]()
                {
                    std::atomic_store(&t, T(3));
                    std::atomic_notify_one(&t);
                });
                std::atomic_wait_explicit(&t, T(1), std::memory_order_seq_cst);
                assert(std::atomic_load(&t) == T(3));
                t1.join();
            }
            {
                volatile A vt(T(2));
                static_assert(noexcept(std::atomic_wait_explicit(&vt, T(0), std::memory_order_seq_cst)), "");
                assert(std::atomic_load(&vt) == T(2));
                std::atomic_wait_explicit(&vt, T(1), std::memory_order_seq_cst);
                std::thread t2 = support::make_test_thread([&]()
                {
                    std::atomic_store(&vt, T(4));
                    std::atomic_notify_one(&vt);
                });
                std::atomic_wait_explicit(&vt, T(2), std::memory_order_seq_cst);
                assert(std::atomic_load(&vt) == T(4));
                t2.join();
            }
        }
    };
}

TEST(atomic_wait_explicit)
{
    TestEachAtomicType<TestWaitExplicit>()();
}

#endif
