#ifdef TEST_HAS_CONTAINERS_ARRAY

#include <array>
#include <memory>
#include <cassert>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/MoveOnly.h"


namespace
{
    struct NoDefault {
        TEST_CONSTEXPR NoDefault(int)
        {}
    };
}

namespace
{
    constexpr bool test_to_array()
    {
        //  Test deduced type.
        {
            auto arr = std::to_array({1, 2, 3});
            ASSERT_SAME_TYPE(decltype(arr), std::array<int, 3>);
            assert(arr[0] == 1);
            assert(arr[1] == 2);
            assert(arr[2] == 3);
        }

        {
            const long l1 = 42;
            auto arr = std::to_array({1L, 4L, 9L, l1});
            ASSERT_SAME_TYPE(decltype(arr)::value_type, long);
            static_assert(arr.size() == 4, "");
            assert(arr[0] == 1);
            assert(arr[1] == 4);
            assert(arr[2] == 9);
            assert(arr[3] == l1);
        }

        {
            auto arr = std::to_array("meow");
            ASSERT_SAME_TYPE(decltype(arr), std::array<char, 5>);
            assert(arr[0] == 'm');
            assert(arr[1] == 'e');
            assert(arr[2] == 'o');
            assert(arr[3] == 'w');
            assert(arr[4] == '\0');
        }

        {
            double source[3] = {4.0, 5.0, 6.0};
            auto arr = std::to_array(source);
            ASSERT_SAME_TYPE(decltype(arr), std::array<double, 3>);
            assert(arr[0] == 4.0);
            assert(arr[1] == 5.0);
            assert(arr[2] == 6.0);
        }

        {
            double source[3] = {4.0, 5.0, 6.0};
            auto arr = std::to_array(std::move(source));
            ASSERT_SAME_TYPE(decltype(arr), std::array<double, 3>);
            assert(arr[0] == 4.0);
            assert(arr[1] == 5.0);
            assert(arr[2] == 6.0);
        }

        {
            MoveOnly source[] = {MoveOnly{0}, MoveOnly{1}, MoveOnly{2}};

            auto arr = std::to_array(std::move(source));
            ASSERT_SAME_TYPE(decltype(arr), std::array<MoveOnly, 3>);
            for (int i = 0; i < 3; ++i)
                assert(arr[i].get() == i && source[i].get() == 0);
        }

    #ifndef _MSVC_STL_VERSION
        // Test C99 compound literal.
        {
            auto arr = std::to_array((int[])
            {
                3, 4
            });
            ASSERT_SAME_TYPE(decltype(arr), std::array<int, 2>);
            assert(arr[0] == 3);
            assert(arr[1] == 4);
        }
    #endif // ! _MSVC_STL_VERSION

        //  Test explicit type.
        {
            auto arr = std::to_array<long>({1, 2, 3});
            ASSERT_SAME_TYPE(decltype(arr), std::array<long, 3>);
            assert(arr[0] == 1);
            assert(arr[1] == 2);
            assert(arr[2] == 3);
        }

        {
            struct A {
                int a;
                double b;
            };

            auto arr = std::to_array<A>({{3, .1}});
            ASSERT_SAME_TYPE(decltype(arr), std::array<A, 1>);
            assert(arr[0].a == 3);
            assert(arr[0].b == .1);
        }

        return true;
    }

    static_assert(test_to_array(), "");

}

TEST(to_array)
{
    test_to_array();
}

namespace
{
    TEST_CONSTEXPR_CXX17 bool test_data()
    {
        {
            typedef double T;
            typedef std::array<T, 3> C;
            C c = {1, 2, 3.5};
            ASSERT_NOEXCEPT(c.data());
            T* p = c.data();
            assert(p[0] == 1);
            assert(p[1] == 2);
            assert(p[2] == 3.5);
        }
        {
            typedef double T;
            typedef std::array<T, 0> C;
            C c = {};
            ASSERT_NOEXCEPT(c.data());
            T* p = c.data();
            (void)p;
        }
        {
            typedef double T;
            typedef std::array<const T, 0> C;
            C c = {{}};
            ASSERT_NOEXCEPT(c.data());
            const T* p = c.data();
            (void)p;
            static_assert((std::is_same<decltype(c.data()), const T*>::value), "");
        }
        {
            typedef NoDefault T;
            typedef std::array<T, 0> C;
            C c = {};
            ASSERT_NOEXCEPT(c.data());
            T* p = c.data();
            (void)p;
        }
        {
            std::array<int, 5> c = {0, 1, 2, 3, 4};
            assert(c.data() == &c[0]);
            assert(*c.data() == c[0]);
        }

        return true;
    }

    static_assert(test_data(), "");

}

TEST(data)
{
    typedef std::max_align_t T;
    typedef std::array<T, 0> C;
    const C c = {};
    const T* p = c.data();
    std::uintptr_t pint = reinterpret_cast<std::uintptr_t>(p);
    assert(pint % TEST_ALIGNOF(T) == 0);
}

namespace
{
    TEST_CONSTEXPR_CXX17 bool test_data_const()
    {
        {
            typedef double T;
            typedef std::array<T, 3> C;
            const C c = {1, 2, 3.5};
            ASSERT_NOEXCEPT(c.data());
            const T* p = c.data();
            assert(p[0] == 1);
            assert(p[1] == 2);
            assert(p[2] == 3.5);
        }
        {
            typedef double T;
            typedef std::array<T, 0> C;
            const C c = {};
            ASSERT_NOEXCEPT(c.data());
            const T* p = c.data();
            (void)p;
        }
        {
            typedef NoDefault T;
            typedef std::array<T, 0> C;
            const C c = {};
            ASSERT_NOEXCEPT(c.data());
            const T* p = c.data();
            (void)p;
        }
        {
            std::array<int, 5> const c = {0, 1, 2, 3, 4};
            assert(c.data() == &c[0]);
            assert(*c.data() == c[0]);
        }

        return true;
    }

    static_assert(test_data_const(), "");

}

TEST(data_const)
{
    typedef std::max_align_t T;
    typedef std::array<T, 0> C;
    const C c = {};
    const T* p = c.data();
    std::uintptr_t pint = reinterpret_cast<std::uintptr_t>(p);
    assert(pint % TEST_ALIGNOF(T) == 0);
}

namespace
{
    TEST_CONSTEXPR_CXX20 bool test_fill()
    {
        {
            typedef double T;
            typedef std::array<T, 3> C;
            C c = {1, 2, 3.5};
            c.fill(5.5);
            assert(c.size() == 3);
            assert(c[0] == 5.5);
            assert(c[1] == 5.5);
            assert(c[2] == 5.5);
        }

        {
            typedef double T;
            typedef std::array<T, 0> C;
            C c = {};
            c.fill(5.5);
            assert(c.size() == 0);
        }
        return true;
    }

    static_assert(test_fill(), "");

}

TEST(fill)
{
    test_fill();
}

TEST(size)
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c = {1, 2, 3.5};
        assert(c.size() == 3);
        assert(c.max_size() == 3);
        assert(!c.empty());
    }
    {
        typedef double T;
        typedef std::array<T, 0> C;
        C c = {};
        assert(c.size() == 0);
        assert(c.max_size() == 0);
        assert(c.empty());
    }
    {
        typedef double T;
        typedef std::array<T, 3> C;
        constexpr C c = {1, 2, 3.5};
        static_assert(c.size() == 3, "");
        static_assert(c.max_size() == 3, "");
        static_assert(!c.empty(), "");
    }
    {
        typedef double T;
        typedef std::array<T, 0> C;
        constexpr C c = {};
        static_assert(c.size() == 0, "");
        static_assert(c.max_size() == 0, "");
        static_assert(c.empty(), "");
    }
}

namespace
{
    struct NonSwappable {
        TEST_CONSTEXPR NonSwappable()
        {}
    private:
        NonSwappable(NonSwappable const&);
        NonSwappable& operator=(NonSwappable const&);
    };

    TEST_CONSTEXPR_CXX20 bool test_swap()
    {
        {
            typedef double T;
            typedef std::array<T, 3> C;
            C c1 = {1, 2, 3.5};
            C c2 = {4, 5, 6.5};
            c1.swap(c2);
            assert(c1.size() == 3);
            assert(c1[0] == 4);
            assert(c1[1] == 5);
            assert(c1[2] == 6.5);
            assert(c2.size() == 3);
            assert(c2[0] == 1);
            assert(c2[1] == 2);
            assert(c2[2] == 3.5);
        }
        {
            typedef double T;
            typedef std::array<T, 3> C;
            C c1 = {1, 2, 3.5};
            C c2 = {4, 5, 6.5};
            std::swap(c1, c2);
            assert(c1.size() == 3);
            assert(c1[0] == 4);
            assert(c1[1] == 5);
            assert(c1[2] == 6.5);
            assert(c2.size() == 3);
            assert(c2[0] == 1);
            assert(c2[1] == 2);
            assert(c2[2] == 3.5);
        }

        {
            typedef double T;
            typedef std::array<T, 0> C;
            C c1 = {};
            C c2 = {};
            c1.swap(c2);
            assert(c1.size() == 0);
            assert(c2.size() == 0);
        }
        {
            typedef double T;
            typedef std::array<T, 0> C;
            C c1 = {};
            C c2 = {};
            std::swap(c1, c2);
            assert(c1.size() == 0);
            assert(c2.size() == 0);
        }
        {
            typedef NonSwappable T;
            typedef std::array<T, 0> C0;
            C0 l = {};
            C0 r = {};
            l.swap(r);
        #if TEST_STD_VER >= 11
            static_assert(noexcept(l.swap(r)), "");
        #endif
        }

        return true;
    }
    static_assert(test_swap(), "");

}

TEST(swap)
{
    test_swap();
}

namespace
{
    template <typename ...T>
    TEST_CONSTEXPR std::array<int, sizeof...(T)> tempArray(T ...args)
    {
        return {args...};
    }

    TEST_CONSTEXPR_CXX14 bool test_get()
    {
        {
            std::array<double, 1> array = {3.3};
            assert(std::get<0>(array) == 3.3);
            std::get<0>(array) = 99.1;
            assert(std::get<0>(array) == 99.1);
        }
        {
            std::array<double, 2> array = {3.3, 4.4};
            assert(std::get<0>(array) == 3.3);
            assert(std::get<1>(array) == 4.4);
            std::get<0>(array) = 99.1;
            std::get<1>(array) = 99.2;
            assert(std::get<0>(array) == 99.1);
            assert(std::get<1>(array) == 99.2);
        }
        {
            std::array<double, 3> array = {3.3, 4.4, 5.5};
            assert(std::get<0>(array) == 3.3);
            assert(std::get<1>(array) == 4.4);
            assert(std::get<2>(array) == 5.5);
            std::get<1>(array) = 99.2;
            assert(std::get<0>(array) == 3.3);
            assert(std::get<1>(array) == 99.2);
            assert(std::get<2>(array) == 5.5);
        }
        {
            std::array<double, 1> array = {3.3};
            static_assert(std::is_same<double&, decltype(std::get<0>(array))>::value, "");
        }
        {
            assert(std::get<0>(tempArray(1, 2, 3)) == 1);
            assert(std::get<1>(tempArray(1, 2, 3)) == 2);
            assert(std::get<2>(tempArray(1, 2, 3)) == 3);
        }

        return true;
    }

    static_assert(test_get(), "");

}

TEST(get)
{
    test_get();
}

namespace
{
    TEST_CONSTEXPR_CXX14 bool test_get_const()
    {
        {
            std::array<double, 1> const array = {3.3};
            assert(std::get<0>(array) == 3.3);
        }
        {
            std::array<double, 2> const array = {3.3, 4.4};
            assert(std::get<0>(array) == 3.3);
            assert(std::get<1>(array) == 4.4);
        }
        {
            std::array<double, 3> const array = {3.3, 4.4, 5.5};
            assert(std::get<0>(array) == 3.3);
            assert(std::get<1>(array) == 4.4);
            assert(std::get<2>(array) == 5.5);
        }
        {
            std::array<double, 1> const array = {3.3};
            static_assert(std::is_same<double const&, decltype(std::get<0>(array))>::value, "");
        }

        return true;
    }

    static_assert(test_get_const(), "");
}

TEST(get_const)
{
    test_get_const();
}

TEST(get_const_rv)
{
    {
        typedef std::unique_ptr<double> T;
        typedef std::array<T, 1> C;
        const C c = {std::unique_ptr<double>(new double(3.5))};
        static_assert(std::is_same<const T&&, decltype(std::get<0>(std::move(c)))>::value, "");
        static_assert(noexcept(std::get<0>(std::move(c))), "");
        const T&& t = std::get<0>(std::move(c));
        assert(*t == 3.5);
    }

#if TEST_STD_VER >= 14
    {
        typedef double T;
        typedef std::array<T, 3> C;
        constexpr const C c = {1, 2, 3.5};
        static_assert(std::get<0>(std::move(c)) == 1, "");
        static_assert(std::get<1>(std::move(c)) == 2, "");
        static_assert(std::get<2>(std::move(c)) == 3.5, "");
    }
#endif
}

TEST(get_rv)
{
    {
        typedef std::unique_ptr<double> T;
        typedef std::array<T, 1> C;
        C c = {std::unique_ptr<double>(new double(3.5))};
        T t = std::get<0>(std::move(c));
        assert(*t == 3.5);
    }
}


#endif
