#ifdef TEST_HAS_CONTAINERS_VIEW_SPAN

#include <cassert>
#include <span>
#include <string>

#include "LLVM.TestSuite/test_macros.h"


namespace
{
    namespace array
    {
        void checkCV()
        {
            int   arr[] = {1, 2, 3};
            const          int  carr[] = {4, 5, 6};
            volatile int  varr[] = {7, 8, 9};
            const volatile int cvarr[] = {1, 3, 5};

            //  Types the same (dynamic sized)
            {
                std::span<               int> s1{arr};    // a span<               int> pointing at int.
                std::span<const          int> s2{carr};    // a span<const          int> pointing at const int.
                std::span<      volatile int> s3{varr};    // a span<      volatile int> pointing at volatile int.
                std::span<const volatile int> s4{cvarr};    // a span<const volatile int> pointing at const volatile int.
                assert(s1.size() + s2.size() + s3.size() + s4.size() == 12);
            }

            //  Types the same (static sized)
            {
                std::span<               int, 3> s1{arr};  // a span<               int> pointing at int.
                std::span<const          int, 3> s2{carr};  // a span<const          int> pointing at const int.
                std::span<      volatile int, 3> s3{varr};  // a span<      volatile int> pointing at volatile int.
                std::span<const volatile int, 3> s4{cvarr};  // a span<const volatile int> pointing at const volatile int.
                assert(s1.size() + s2.size() + s3.size() + s4.size() == 12);
            }


            //  types different (dynamic sized)
            {
                std::span<const          int> s1{arr};     // a span<const          int> pointing at int.
                std::span<      volatile int> s2{arr};     // a span<      volatile int> pointing at int.
                std::span<      volatile int> s3{arr};     // a span<      volatile int> pointing at const int.
                std::span<const volatile int> s4{arr};     // a span<const volatile int> pointing at int.
                std::span<const volatile int> s5{carr};     // a span<const volatile int> pointing at const int.
                std::span<const volatile int> s6{varr};     // a span<const volatile int> pointing at volatile int.
                assert(s1.size() + s2.size() + s3.size() + s4.size() + s5.size() + s6.size() == 18);
            }

            //  types different (static sized)
            {
                std::span<const          int, 3> s1{arr};   // a span<const          int> pointing at int.
                std::span<      volatile int, 3> s2{arr};   // a span<      volatile int> pointing at int.
                std::span<      volatile int, 3> s3{arr};   // a span<      volatile int> pointing at const int.
                std::span<const volatile int, 3> s4{arr};   // a span<const volatile int> pointing at int.
                std::span<const volatile int, 3> s5{carr};   // a span<const volatile int> pointing at const int.
                std::span<const volatile int, 3> s6{varr};   // a span<const volatile int> pointing at volatile int.
                assert(s1.size() + s2.size() + s3.size() + s4.size() + s5.size() + s6.size() == 18);
            }
        }

        template<class T>
        constexpr bool testSpan()
        {
            T val[2] = {};

            ASSERT_NOEXCEPT(std::span<T>{val});
            ASSERT_NOEXCEPT(std::span<T, 2>{val});
            ASSERT_NOEXCEPT(std::span<const T>{val});
            ASSERT_NOEXCEPT(std::span<const T, 2>{val});

            std::span<T> s1 = val;
            std::span<T, 2> s2 = val;
            std::span<const T> s3 = val;
            std::span<const T, 2> s4 = val;
            assert(s1.data() == val && s1.size() == 2);
            assert(s2.data() == val && s2.size() == 2);
            assert(s3.data() == val && s3.size() == 2);
            assert(s4.data() == val && s4.size() == 2);

            std::span<const int> s5 = {{1, 2}};
        #if TEST_STD_VER >= 26
            std::span<const int, 2> s6({1, 2});
        #else
            std::span<const int, 2> s6 = {{1, 2}};
        #endif
            assert(s5.size() == 2);  // and it dangles
            assert(s6.size() == 2);  // and it dangles

            return true;
        }
    }
}

TEST(array)
{
    using namespace array;
    struct A {};

    testSpan<int>();
    testSpan<double>();
    testSpan<A>();
    testSpan<std::string>();

    static_assert(testSpan<int>());
    static_assert(testSpan<double>());
    static_assert(testSpan<A>());

    checkCV();

    // Size wrong
    {
        static_assert(!std::is_constructible<std::span<int, 2>, int(&)[3]>::value, "");
    }

    // Type wrong
    {
        static_assert(!std::is_constructible<std::span<float>, int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<float, 3>, int(&)[3]>::value, "");
    }

    // CV wrong (dynamically sized)
    {
        static_assert(!std::is_constructible<std::span<int>, const int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<int>, volatile int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<int>, const volatile int(&)[3]>::value, "");

        static_assert(!std::is_constructible<std::span<const int>, volatile int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<const int>, const volatile int(&)[3]>::value, "");

        static_assert(!std::is_constructible<std::span<volatile int>, const int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<volatile int>, const volatile int(&)[3]>::value, "");
    }

    // CV wrong (statically sized)
    {
        static_assert(!std::is_constructible<std::span<int, 3>, const int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<int, 3>, volatile int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<int, 3>, const volatile int(&)[3]>::value, "");

        static_assert(!std::is_constructible<std::span<const int, 3>, volatile int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<const int, 3>, const volatile int(&)[3]>::value, "");

        static_assert(!std::is_constructible<std::span<volatile int, 3>, const int(&)[3]>::value, "");
        static_assert(!std::is_constructible<std::span<volatile int, 3>, const volatile int(&)[3]>::value, "");
    }
}

namespace
{
    namespace assign
    {
        template <typename T>
        constexpr bool doAssign(T lhs, T rhs)
        {
            ASSERT_NOEXCEPT(std::declval<T&>() = rhs);
            lhs = rhs;
            return lhs.data() == rhs.data()
                && lhs.size() == rhs.size();
        }
    }
}

TEST(assign)
{
    using namespace assign;
    struct A {};

    constexpr int carr1[] = {1, 2, 3, 4};
    constexpr int carr2[] = {3, 4, 5};
    constexpr int carr3[] = {7, 8};
    int   arr[] = {5, 6, 7, 9};
    std::string strs[] = {"ABC", "DEF", "GHI"};

    //  dynamically sized assignment
    {
        std::span<int> spans[] = {
            {},
            {arr, arr + 1},
            {arr, arr + 2},
            {arr, arr + 3},
            {arr + 1, arr + 3} // same size as s2
        };

        for (std::size_t i = 0; i < std::size(spans); ++i)
            for (std::size_t j = i; j < std::size(spans); ++j)
                assert((doAssign(spans[i], spans[j])));
    }

    //  statically sized assignment
    {
        using spanType = std::span<int, 2>;
        spanType spans[] = {
            spanType{arr, arr + 2},
            spanType{arr + 1, arr + 3},
            spanType{arr + 2, arr + 4}
        };

        for (std::size_t i = 0; i < std::size(spans); ++i)
            for (std::size_t j = i; j < std::size(spans); ++j)
                assert((doAssign(spans[i], spans[j])));
    }

    //  dynamically sized assignment
    {
        std::span<std::string> spans[] = {
            {strs, strs},
            {strs, strs + 1},
            {strs, strs + 2},
            {strs, strs + 3},
            {strs + 1, strs + 1},
            {strs + 1, strs + 2},
            {strs + 1, strs + 3},
            {strs + 2, strs + 2},
            {strs + 2, strs + 3},
            {strs + 3, strs + 3}
        };

        for (std::size_t i = 0; i < std::size(spans); ++i)
            for (std::size_t j = i; j < std::size(spans); ++j)
                assert((doAssign(spans[i], spans[j])));
    }

    {
        using spanType = std::span<std::string, 1>;
        spanType spans[] = {
            spanType{strs, strs + 1},
            spanType{strs + 1, strs + 2},
            spanType{strs + 2, strs + 3}
        };

        for (std::size_t i = 0; i < std::size(spans); ++i)
            for (std::size_t j = i; j < std::size(spans); ++j)
                assert((doAssign(spans[i], spans[j])));
    }
}

namespace
{
    namespace copy
    {
        template <typename T>
        constexpr bool doCopy(const T& rhs)
        {
            ASSERT_NOEXCEPT(T{rhs});
            T lhs{rhs};
            return lhs.data() == rhs.data()
                && lhs.size() == rhs.size();
        }

        struct A {};

        template <typename T>
        void testCV()
        {
            int  arr[] = {1, 2, 3};
            assert((doCopy(std::span<T>())));
            assert((doCopy(std::span<T, 0>())));
            assert((doCopy(std::span<T>(&arr[0], 1))));
            assert((doCopy(std::span<T, 1>(&arr[0], 1))));
            assert((doCopy(std::span<T>(&arr[0], 2))));
            assert((doCopy(std::span<T, 2>(&arr[0], 2))));
        }
    }
}

TEST(copy)
{
    using namespace copy;

    constexpr int carr[] = {1, 2, 3};

    static_assert(doCopy(std::span<      int>()), "");
    static_assert(doCopy(std::span<      int, 0>()), "");
    static_assert(doCopy(std::span<const int>(&carr[0], 1)), "");
    static_assert(doCopy(std::span<const int, 1>(&carr[0], 1)), "");
    static_assert(doCopy(std::span<const int>(&carr[0], 2)), "");
    static_assert(doCopy(std::span<const int, 2>(&carr[0], 2)), "");

    static_assert(doCopy(std::span<long>()), "");
    static_assert(doCopy(std::span<double>()), "");
    static_assert(doCopy(std::span<A>()), "");

    std::string s;
    assert(doCopy(std::span<std::string>()));
    assert(doCopy(std::span<std::string, 0>()));
    assert(doCopy(std::span<std::string>(&s, 1)));
    assert(doCopy(std::span<std::string, 1>(&s, 1)));

    testCV<               int>();
    testCV<const          int>();
    testCV<      volatile int>();
    testCV<const volatile int>();
}

namespace
{
    namespace span
    {
        template <class T, class From>
        TEST_CONSTEXPR_CXX20 void check()
        {
            // dynamic -> dynamic
            {
                {
                    std::span<From> from;
                    std::span<T> span{from};
                    ASSERT_NOEXCEPT(std::span<T>(from));
                    assert(span.data() == nullptr);
                    assert(span.size() == 0);
                }
                {
                    From array[3] = {};
                    std::span<From> from(array);
                    std::span<T> span{from};
                    ASSERT_NOEXCEPT(std::span<T>(from));
                    assert(span.data() == array);
                    assert(span.size() == 3);
                }
            }

            // static -> static
            {
                {
                    std::span<From, 0> from;
                    std::span<T, 0> span{from};
                    ASSERT_NOEXCEPT(std::span<T, 0>(from));
                    assert(span.data() == nullptr);
                    assert(span.size() == 0);
                }

                {
                    From array[3] = {};
                    std::span<From, 3> from(array);
                    std::span<T, 3> span{from};
                    ASSERT_NOEXCEPT(std::span<T, 3>(from));
                    assert(span.data() == array);
                    assert(span.size() == 3);
                }
            }

            // static -> dynamic
            {
                {
                    std::span<From, 0> from;
                    std::span<T> span{from};
                    ASSERT_NOEXCEPT(std::span<T>(from));
                    assert(span.data() == nullptr);
                    assert(span.size() == 0);
                }

                {
                    From array[3] = {};
                    std::span<From, 3> from(array);
                    std::span<T> span{from};
                    ASSERT_NOEXCEPT(std::span<T>(from));
                    assert(span.data() == array);
                    assert(span.size() == 3);
                }
            }

            // dynamic -> static (not allowed)
        }

        template <class T>
        TEST_CONSTEXPR_CXX20 void check_cvs()
        {
            check<T, T>();

            check<T const, T>();
            check<T const, T const>();

            check<T volatile, T>();
            check<T volatile, T volatile>();

            check<T const volatile, T>();
            check<T const volatile, T const>();
            check<T const volatile, T volatile>();
            check<T const volatile, T const volatile>();
        }

        struct A {};
    }
}

TEST(span)
{
    using namespace span;

    check_cvs<int>();
    check_cvs<long>();
    check_cvs<double>();
    check_cvs<std::string>();
    check_cvs<A>();
    check_cvs<int>();
    check_cvs<long>();
    check_cvs<double>();
    check_cvs<std::string>();
    check_cvs<A>();
}

namespace
{
    namespace back
    {
        template <typename Span>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT(noexcept(sp.back()));
            return std::addressof(sp.back()) == sp.data() + sp.size() - 1;
        }

        template <typename Span>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT(noexcept(sp.back()));
            assert(std::addressof(sp.back()) == sp.data() + sp.size() - 1);
        }

        template <typename Span>
        void testEmptySpan(Span sp)
        {
            if (!sp.empty())
                [[maybe_unused]] auto res = sp.back();
        }

        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    }

}

TEST(back)
{
    using namespace back;

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4)), "");


    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));


    testRuntimeSpan(std::span<int, 1>(iArr2, 1));
    testRuntimeSpan(std::span<int, 2>(iArr2, 2));
    testRuntimeSpan(std::span<int, 3>(iArr2, 3));
    testRuntimeSpan(std::span<int, 4>(iArr2, 4));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, 1));
    testRuntimeSpan(std::span<std::string, 1>(&s, 1));

    std::span<int, 0> sp;
    testEmptySpan(sp);
}

namespace
{
    namespace data
    {
        template <typename Span>
        constexpr bool testConstexprSpan(Span sp, typename Span::pointer ptr)
        {
            ASSERT_NOEXCEPT(sp.data());
            return sp.data() == ptr;
        }


        template <typename Span>
        void testRuntimeSpan(Span sp, typename Span::pointer ptr)
        {
            ASSERT_NOEXCEPT(sp.data());
            assert(sp.data() == ptr);
        }

        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(data)
{
    using namespace data;

    //  dynamic size
    static_assert(testConstexprSpan(std::span<int>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<long>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<double>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<A>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<std::string>(), nullptr), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), iArr1), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 1, 1), iArr1 + 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 2, 2), iArr1 + 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 3, 3), iArr1 + 3), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1 + 4, 4), iArr1 + 4), "");

    //  static size
    static_assert(testConstexprSpan(std::span<int, 0>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<long, 0>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<double, 0>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<A, 0>(), nullptr), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>(), nullptr), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), iArr1), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), iArr1), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1 + 1, 1), iArr1 + 1), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1 + 2, 2), iArr1 + 2), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1 + 3, 3), iArr1 + 3), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1 + 4, 4), iArr1 + 4), "");


    //  dynamic size
    testRuntimeSpan(std::span<int>(), nullptr);
    testRuntimeSpan(std::span<long>(), nullptr);
    testRuntimeSpan(std::span<double>(), nullptr);
    testRuntimeSpan(std::span<A>(), nullptr);
    testRuntimeSpan(std::span<std::string>(), nullptr);

    testRuntimeSpan(std::span<int>(iArr2, 1), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 2), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 3), iArr2);
    testRuntimeSpan(std::span<int>(iArr2, 4), iArr2);

    testRuntimeSpan(std::span<int>(iArr2 + 1, 1), iArr2 + 1);
    testRuntimeSpan(std::span<int>(iArr2 + 2, 2), iArr2 + 2);
    testRuntimeSpan(std::span<int>(iArr2 + 3, 3), iArr2 + 3);
    testRuntimeSpan(std::span<int>(iArr2 + 4, 4), iArr2 + 4);

    //  static size
    testRuntimeSpan(std::span<int, 0>(), nullptr);
    testRuntimeSpan(std::span<long, 0>(), nullptr);
    testRuntimeSpan(std::span<double, 0>(), nullptr);
    testRuntimeSpan(std::span<A, 0>(), nullptr);
    testRuntimeSpan(std::span<std::string, 0>(), nullptr);

    testRuntimeSpan(std::span<int, 1>(iArr2, 1), iArr2);
    testRuntimeSpan(std::span<int, 2>(iArr2, 2), iArr2);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), iArr2);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), iArr2);

    testRuntimeSpan(std::span<int, 1>(iArr2 + 1, 1), iArr2 + 1);
    testRuntimeSpan(std::span<int, 2>(iArr2 + 2, 2), iArr2 + 2);
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3), iArr2 + 3);
    testRuntimeSpan(std::span<int, 4>(iArr2 + 4, 4), iArr2 + 4);


    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, 1), &s);
    testRuntimeSpan(std::span<std::string, 1>(&s, 1), &s);
}

namespace
{
    namespace front
    {
        template <typename Span>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT(noexcept(sp.front()));
            return std::addressof(sp.front()) == sp.data();
        }


        template <typename Span>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT(noexcept(sp.front()));
            assert(std::addressof(sp.front()) == sp.data());
        }

        template <typename Span>
        void testEmptySpan(Span sp)
        {
            if (!sp.empty())
                [[maybe_unused]] auto res = sp.front();
        }

        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(front)
{
    using namespace front;

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");

    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4)), "");


    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));


    testRuntimeSpan(std::span<int, 1>(iArr2, 1));
    testRuntimeSpan(std::span<int, 2>(iArr2, 2));
    testRuntimeSpan(std::span<int, 3>(iArr2, 3));
    testRuntimeSpan(std::span<int, 4>(iArr2, 4));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, 1));
    testRuntimeSpan(std::span<std::string, 1>(&s, 1));

    std::span<int, 0> sp;
    testEmptySpan(sp);
}

namespace
{
    namespace op_idx
    {
        template <typename Span>
        constexpr bool testConstexprSpan(Span sp, std::size_t idx)
        {
            LIBCPP_ASSERT(noexcept(sp[idx]));

            typename Span::reference r1 = sp[idx];
            typename Span::reference r2 = *(sp.data() + idx);
            return r1 == r2;
        }


        template <typename Span>
        void testRuntimeSpan(Span sp, std::size_t idx)
        {
            LIBCPP_ASSERT(noexcept(sp[idx]));

            typename Span::reference r1 = sp[idx];
            typename Span::reference r2 = *(sp.data() + idx);
            assert(r1 == r2);
        }

        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(op_idx)
{
    using namespace op_idx;

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), 0), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 1), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 2), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 0), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 3), "");


    static_assert(testConstexprSpan(std::span<const int, 1>(iArr1, 1), 0), "");

    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 2>(iArr1, 2), 1), "");

    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 1), "");
    static_assert(testConstexprSpan(std::span<const int, 3>(iArr1, 3), 2), "");

    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 0), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 1), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 2), "");
    static_assert(testConstexprSpan(std::span<const int, 4>(iArr1, 4), 3), "");


    testRuntimeSpan(std::span<int>(iArr2, 1), 0);

    testRuntimeSpan(std::span<int>(iArr2, 2), 0);
    testRuntimeSpan(std::span<int>(iArr2, 2), 1);

    testRuntimeSpan(std::span<int>(iArr2, 3), 0);
    testRuntimeSpan(std::span<int>(iArr2, 3), 1);
    testRuntimeSpan(std::span<int>(iArr2, 3), 2);

    testRuntimeSpan(std::span<int>(iArr2, 4), 0);
    testRuntimeSpan(std::span<int>(iArr2, 4), 1);
    testRuntimeSpan(std::span<int>(iArr2, 4), 2);
    testRuntimeSpan(std::span<int>(iArr2, 4), 3);


    testRuntimeSpan(std::span<int, 1>(iArr2, 1), 0);

    testRuntimeSpan(std::span<int, 2>(iArr2, 2), 0);
    testRuntimeSpan(std::span<int, 2>(iArr2, 2), 1);

    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 0);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 1);
    testRuntimeSpan(std::span<int, 3>(iArr2, 3), 2);

    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 0);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 1);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 2);
    testRuntimeSpan(std::span<int, 4>(iArr2, 4), 3);

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, 1), 0);
    testRuntimeSpan(std::span<std::string, 1>(&s, 1), 0);

}

namespace
{
    namespace begin
    {
        template <class Span>
        constexpr bool testConstexprSpan(Span s)
        {
            bool ret = true;
            typename Span::iterator b = s.begin();

            if (s.empty()) {
                ret = ret && (b == s.end());
            }
            else {
                ret = ret && (*b == s[0]);
                ret = ret && (&*b == &s[0]);
            }
            return ret;
        }


        template <class Span>
        void testRuntimeSpan(Span s)
        {
            typename Span::iterator b = s.begin();

            if (s.empty()) {
                assert(b == s.end());
            }
            else {
                assert(*b == s[0]);
                assert(&*b == &s[0]);
            }
        }

        struct A {};
        bool operator==(A, A)
        {
            return true;
        }

        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(begin)
{
    using namespace begin;

    static_assert(testConstexprSpan(std::span<int>()), "");
    static_assert(testConstexprSpan(std::span<long>()), "");
    static_assert(testConstexprSpan(std::span<double>()), "");
    static_assert(testConstexprSpan(std::span<A>()), "");
    static_assert(testConstexprSpan(std::span<std::string>()), "");

    static_assert(testConstexprSpan(std::span<int, 0>()), "");
    static_assert(testConstexprSpan(std::span<long, 0>()), "");
    static_assert(testConstexprSpan(std::span<double, 0>()), "");
    static_assert(testConstexprSpan(std::span<A, 0>()), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>()), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5)), "");


    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace end
    {
        template <class Span>
        constexpr bool testConstexprSpan(Span s)
        {
            bool ret = true;
            typename Span::iterator e = s.end();
            if (s.empty()) {
                ret = ret && (e == s.begin());
            }
            else {
                typename Span::const_pointer last = &*(s.begin() + s.size() - 1);
                ret = ret && (e != s.begin());
                ret = ret && (&*(e - 1) == last);
            }

            ret = ret && (static_cast<std::size_t>(e - s.begin()) == s.size());
            return ret;
        }

        template <class Span>
        void testRuntimeSpan(Span s)
        {
            typename Span::iterator e = s.end();
            if (s.empty()) {
                assert(e == s.begin());
            }
            else {
                typename Span::const_pointer last = &*(s.begin() + s.size() - 1);
                assert(e != s.begin());
                assert(&*(e - 1) == last);
            }

            assert(static_cast<std::size_t>(e - s.begin()) == s.size());
        }


        struct A {};
        bool operator==(A, A)
        {
            return true;
        }

        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(end)
{
    using namespace end;

    static_assert(testConstexprSpan(std::span<int>()), "");
    static_assert(testConstexprSpan(std::span<long>()), "");
    static_assert(testConstexprSpan(std::span<double>()), "");
    static_assert(testConstexprSpan(std::span<A>()), "");
    static_assert(testConstexprSpan(std::span<std::string>()), "");

    static_assert(testConstexprSpan(std::span<int, 0>()), "");
    static_assert(testConstexprSpan(std::span<long, 0>()), "");
    static_assert(testConstexprSpan(std::span<double, 0>()), "");
    static_assert(testConstexprSpan(std::span<A, 0>()), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>()), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5)), "");


    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace rbegin
    {
        template <class Span>
        constexpr bool testConstexprSpan(Span s)
        {
            bool ret = true;
            typename Span::reverse_iterator b = s.rbegin();
            if (s.empty()) {
                ret = ret && (b == s.rend());
            }
            else {
                const typename Span::size_type last = s.size() - 1;
                ret = ret && (*b == s[last]);
                ret = ret && (&*b == &s[last]);
            }
            return ret;
        }


        template <class Span>
        void testRuntimeSpan(Span s)
        {
            typename Span::reverse_iterator b = s.rbegin();
            if (s.empty()) {
                assert(b == s.rend());
            }
            else {
                const typename Span::size_type last = s.size() - 1;
                assert(*b == s[last]);
                assert(&*b == &s[last]);
            }
        }


        struct A {};
        bool operator==(A, A)
        {
            return true;
        }

        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(rbegin)
{
    using namespace rbegin;

    static_assert(testConstexprSpan(std::span<int>()), "");
    static_assert(testConstexprSpan(std::span<long>()), "");
    static_assert(testConstexprSpan(std::span<double>()), "");
    static_assert(testConstexprSpan(std::span<A>()), "");
    static_assert(testConstexprSpan(std::span<std::string>()), "");

    static_assert(testConstexprSpan(std::span<int, 0>()), "");
    static_assert(testConstexprSpan(std::span<long, 0>()), "");
    static_assert(testConstexprSpan(std::span<double, 0>()), "");
    static_assert(testConstexprSpan(std::span<A, 0>()), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>()), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5)), "");

    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, static_cast<std::size_t>(0)));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace rend
    {
        template <class Span>
        constexpr bool testConstexprSpan(Span s)
        {
            bool ret = true;
            typename Span::reverse_iterator e = s.rend();
            if (s.empty()) {
                ret = ret && (e == s.rbegin());
            }
            else {
                ret = ret && (e != s.rbegin());
            }

            ret = ret && (static_cast<std::size_t>(e - s.rbegin()) == s.size());
            return ret;
        }

        template <class Span>
        void testRuntimeSpan(Span s)
        {
            typename Span::reverse_iterator e = s.rend();
            if (s.empty()) {
                assert(e == s.rbegin());
            }
            else {
                assert(e != s.rbegin());
            }

            assert(static_cast<std::size_t>(e - s.rbegin()) == s.size());
        }


        struct A {};
        bool operator==(A, A)
        {
            return true;
        }

        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(rend)
{
    using namespace rend;

    static_assert(testConstexprSpan(std::span<int>()), "");
    static_assert(testConstexprSpan(std::span<long>()), "");
    static_assert(testConstexprSpan(std::span<double>()), "");
    static_assert(testConstexprSpan(std::span<A>()), "");
    static_assert(testConstexprSpan(std::span<std::string>()), "");

    static_assert(testConstexprSpan(std::span<int, 0>()), "");
    static_assert(testConstexprSpan(std::span<long, 0>()), "");
    static_assert(testConstexprSpan(std::span<double, 0>()), "");
    static_assert(testConstexprSpan(std::span<A, 0>()), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>()), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4)), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5)), "");


    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace as_bytes
    {
        template<typename Span>
        void testRuntimeSpan(Span sp)
        {
            ASSERT_NOEXCEPT(std::as_bytes(sp));

            auto spBytes = std::as_bytes(sp);
            using SB = decltype(spBytes);
            ASSERT_SAME_TYPE(const std::byte, typename SB::element_type);

            if constexpr (sp.extent == std::dynamic_extent)
                assert(spBytes.extent == std::dynamic_extent);
            else
                assert(spBytes.extent == sizeof(typename Span::element_type) * sp.extent);

            assert((void*)spBytes.data() == (void*)sp.data());
            assert(spBytes.size() == sp.size_bytes());
        }

        struct A {};
        int iArr2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    }
}

TEST(as_bytes)
{
    using namespace as_bytes;

    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1));
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2));
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3));
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4));
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace as_writable_bytes
    {
        template<typename Span>
        void testRuntimeSpan(Span sp)
        {
            ASSERT_NOEXCEPT(std::as_writable_bytes(sp));

            auto spBytes = std::as_writable_bytes(sp);
            using SB = decltype(spBytes);
            ASSERT_SAME_TYPE(std::byte, typename SB::element_type);

            if constexpr (sp.extent == std::dynamic_extent)
                assert(spBytes.extent == std::dynamic_extent);
            else
                assert(spBytes.extent == sizeof(typename Span::element_type) * sp.extent);

            assert(static_cast<void*>(spBytes.data()) == static_cast<void*>(sp.data()));
            assert(spBytes.size() == sp.size_bytes());
        }

        struct A {};
        int iArr2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    }
}

TEST(as_writable_bytes)
{
    using namespace as_writable_bytes;

    testRuntimeSpan(std::span<int>());
    testRuntimeSpan(std::span<long>());
    testRuntimeSpan(std::span<double>());
    testRuntimeSpan(std::span<A>());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>());
    testRuntimeSpan(std::span<long, 0>());
    testRuntimeSpan(std::span<double, 0>());
    testRuntimeSpan(std::span<A, 0>());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1));
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2));
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3));
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4));
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}

namespace
{
    namespace empty
    {
        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    }
}

TEST(empty)
{
    using namespace empty;

    static_assert(noexcept(std::span<int>().empty()), "");
    static_assert(noexcept(std::span<int, 0>().empty()), "");


    static_assert(std::span<int>().empty(), "");
    static_assert(std::span<long>().empty(), "");
    static_assert(std::span<double>().empty(), "");
    static_assert(std::span<A>().empty(), "");
    static_assert(std::span<std::string>().empty(), "");

    static_assert(std::span<int, 0>().empty(), "");
    static_assert(std::span<long, 0>().empty(), "");
    static_assert(std::span<double, 0>().empty(), "");
    static_assert(std::span<A, 0>().empty(), "");
    static_assert(std::span<std::string, 0>().empty(), "");

    static_assert(!std::span<const int>(iArr1, 1).empty(), "");
    static_assert(!std::span<const int>(iArr1, 2).empty(), "");
    static_assert(!std::span<const int>(iArr1, 3).empty(), "");
    static_assert(!std::span<const int>(iArr1, 4).empty(), "");
    static_assert(!std::span<const int>(iArr1, 5).empty(), "");

    assert((std::span<int>().empty()));
    assert((std::span<long>().empty()));
    assert((std::span<double>().empty()));
    assert((std::span<A>().empty()));
    assert((std::span<std::string>().empty()));

    assert((std::span<int, 0>().empty()));
    assert((std::span<long, 0>().empty()));
    assert((std::span<double, 0>().empty()));
    assert((std::span<A, 0>().empty()));
    assert((std::span<std::string, 0>().empty()));

    assert(!(std::span<int, 1>(iArr2, 1).empty()));
    assert(!(std::span<int, 2>(iArr2, 2).empty()));
    assert(!(std::span<int, 3>(iArr2, 3).empty()));
    assert(!(std::span<int, 4>(iArr2, 4).empty()));
    assert(!(std::span<int, 5>(iArr2, 5).empty()));

    std::string s;
    assert(((std::span<std::string>(&s, (std::size_t)0)).empty()));
    assert(!((std::span<std::string>(&s, 1).empty())));
}

namespace
{
    namespace size
    {

        template <typename Span>
        constexpr bool testConstexprSpan(Span sp, std::size_t sz)
        {
            ASSERT_NOEXCEPT(sp.size());
            return sp.size() == sz;
        }


        template <typename Span>
        void testRuntimeSpan(Span sp, std::size_t sz)
        {
            ASSERT_NOEXCEPT(sp.size());
            assert(sp.size() == sz);
        }

        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(size)
{
    using namespace size;

    static_assert(testConstexprSpan(std::span<int>(), 0), "");
    static_assert(testConstexprSpan(std::span<long>(), 0), "");
    static_assert(testConstexprSpan(std::span<double>(), 0), "");
    static_assert(testConstexprSpan(std::span<A>(), 0), "");
    static_assert(testConstexprSpan(std::span<std::string>(), 0), "");

    static_assert(testConstexprSpan(std::span<int, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<long, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<double, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<A, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>(), 0), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 3), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 4), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5), 5), "");

    testRuntimeSpan(std::span<int>(), 0);
    testRuntimeSpan(std::span<long>(), 0);
    testRuntimeSpan(std::span<double>(), 0);
    testRuntimeSpan(std::span<A>(), 0);
    testRuntimeSpan(std::span<std::string>(), 0);

    testRuntimeSpan(std::span<int, 0>(), 0);
    testRuntimeSpan(std::span<long, 0>(), 0);
    testRuntimeSpan(std::span<double, 0>(), 0);
    testRuntimeSpan(std::span<A, 0>(), 0);
    testRuntimeSpan(std::span<std::string, 0>(), 0);

    testRuntimeSpan(std::span<int>(iArr2, 1), 1);
    testRuntimeSpan(std::span<int>(iArr2, 2), 2);
    testRuntimeSpan(std::span<int>(iArr2, 3), 3);
    testRuntimeSpan(std::span<int>(iArr2, 4), 4);
    testRuntimeSpan(std::span<int>(iArr2, 5), 5);

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1), 1);
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2), 2);
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3), 3);
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4), 4);
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5), 5);

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0), 0);
    testRuntimeSpan(std::span<std::string>(&s, 1), 1);
}

namespace
{
    namespace size_bytes
    {
        template <typename Span>
        constexpr bool testConstexprSpan(Span sp, std::size_t sz)
        {
            ASSERT_NOEXCEPT(sp.size_bytes());
            return (std::size_t)sp.size_bytes() == sz * sizeof(typename Span::element_type);
        }


        template <typename Span>
        void testRuntimeSpan(Span sp, std::size_t sz)
        {
            ASSERT_NOEXCEPT(sp.size_bytes());
            assert((std::size_t)sp.size_bytes() == sz * sizeof(typename Span::element_type));
        }

        struct A {};
        constexpr int iArr1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int iArr2[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    }
}

TEST(size_bytes)
{
    using namespace size_bytes;

    static_assert(testConstexprSpan(std::span<int>(), 0), "");
    static_assert(testConstexprSpan(std::span<long>(), 0), "");
    static_assert(testConstexprSpan(std::span<double>(), 0), "");
    static_assert(testConstexprSpan(std::span<A>(), 0), "");
    static_assert(testConstexprSpan(std::span<std::string>(), 0), "");

    static_assert(testConstexprSpan(std::span<int, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<long, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<double, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<A, 0>(), 0), "");
    static_assert(testConstexprSpan(std::span<std::string, 0>(), 0), "");

    static_assert(testConstexprSpan(std::span<const int>(iArr1, 1), 1), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 2), 2), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 3), 3), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 4), 4), "");
    static_assert(testConstexprSpan(std::span<const int>(iArr1, 5), 5), "");

    testRuntimeSpan(std::span<int>(), 0);
    testRuntimeSpan(std::span<long>(), 0);
    testRuntimeSpan(std::span<double>(), 0);
    testRuntimeSpan(std::span<A>(), 0);
    testRuntimeSpan(std::span<std::string>(), 0);

    testRuntimeSpan(std::span<int, 0>(), 0);
    testRuntimeSpan(std::span<long, 0>(), 0);
    testRuntimeSpan(std::span<double, 0>(), 0);
    testRuntimeSpan(std::span<A, 0>(), 0);
    testRuntimeSpan(std::span<std::string, 0>(), 0);

    testRuntimeSpan(std::span<int>(iArr2, 1), 1);
    testRuntimeSpan(std::span<int>(iArr2, 2), 2);
    testRuntimeSpan(std::span<int>(iArr2, 3), 3);
    testRuntimeSpan(std::span<int>(iArr2, 4), 4);
    testRuntimeSpan(std::span<int>(iArr2, 5), 5);

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1), 1);
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2), 2);
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3), 3);
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4), 4);
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5), 5);

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::size_t)0), 0);
    testRuntimeSpan(std::span<std::string>(&s, 1), 1);
}

namespace
{
    namespace first
    {
        template <typename Span, std::size_t Count>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template first<Count>())));
            LIBCPP_ASSERT((noexcept(sp.first(Count))));
            auto s1 = sp.template first<Count>();
            auto s2 = sp.first(Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count, "");
            static_assert(S2::extent == std::dynamic_extent, "");
            return
                s1.data() == s2.data()
                && s1.size() == s2.size()
                && std::equal(s1.begin(), s1.end(), sp.begin());
        }


        template <typename Span, std::size_t Count>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template first<Count>())));
            LIBCPP_ASSERT((noexcept(sp.first(Count))));
            auto s1 = sp.template first<Count>();
            auto s2 = sp.first(Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count, "");
            static_assert(S2::extent == std::dynamic_extent, "");
            assert(s1.data() == s2.data());
            assert(s1.size() == s2.size());
            assert(std::equal(s1.begin(), s1.end(), sp.begin()));
        }


        constexpr int carr1[] = {1, 2, 3, 4};
        int   arr[] = {5, 6, 7};
        std::string   sarr[] = {"ABC", "DEF", "GHI", "JKL", "MNO"};

    }
}

TEST(first)
{
    using namespace first;

    {
        using Sp = std::span<const int>;
        static_assert(testConstexprSpan<Sp, 0>(Sp{}), "");

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<const int, 4>;

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<int>;
        testRuntimeSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0>(Sp{arr});
        testRuntimeSpan<Sp, 1>(Sp{arr});
        testRuntimeSpan<Sp, 2>(Sp{arr});
        testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
        using Sp = std::span<int, 3>;

        testRuntimeSpan<Sp, 0>(Sp{arr});
        testRuntimeSpan<Sp, 1>(Sp{arr});
        testRuntimeSpan<Sp, 2>(Sp{arr});
        testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
        using Sp = std::span<std::string>;
        testConstexprSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0>(Sp{sarr});
        testRuntimeSpan<Sp, 1>(Sp{sarr});
        testRuntimeSpan<Sp, 2>(Sp{sarr});
        testRuntimeSpan<Sp, 3>(Sp{sarr});
        testRuntimeSpan<Sp, 4>(Sp{sarr});
        testRuntimeSpan<Sp, 5>(Sp{sarr});
    }

    {
        using Sp = std::span<std::string, 5>;

        testRuntimeSpan<Sp, 0>(Sp{sarr});
        testRuntimeSpan<Sp, 1>(Sp{sarr});
        testRuntimeSpan<Sp, 2>(Sp{sarr});
        testRuntimeSpan<Sp, 3>(Sp{sarr});
        testRuntimeSpan<Sp, 4>(Sp{sarr});
        testRuntimeSpan<Sp, 5>(Sp{sarr});
    }
}

namespace
{
    namespace last
    {
        template <typename Span, std::size_t Count>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template last<Count>())));
            LIBCPP_ASSERT((noexcept(sp.last(Count))));
            auto s1 = sp.template last<Count>();
            auto s2 = sp.last(Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count, "");
            static_assert(S2::extent == std::dynamic_extent, "");
            return
                s1.data() == s2.data()
                && s1.size() == s2.size()
                && std::equal(s1.begin(), s1.end(), sp.end() - Count);
        }


        template <typename Span, std::size_t Count>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template last<Count>())));
            LIBCPP_ASSERT((noexcept(sp.last(Count))));
            auto s1 = sp.template last<Count>();
            auto s2 = sp.last(Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count, "");
            static_assert(S2::extent == std::dynamic_extent, "");
            assert(s1.data() == s2.data());
            assert(s1.size() == s2.size());
            assert(std::equal(s1.begin(), s1.end(), sp.end() - Count));
        }


        constexpr int carr1[] = {1, 2, 3, 4};
        int   arr[] = {5, 6, 7};
        std::string   sarr[] = {"ABC", "DEF", "GHI", "JKL", "MNO"};


    }
}

TEST(last)
{
    using namespace last;

    {
        using Sp = std::span<const int>;
        static_assert(testConstexprSpan<Sp, 0>(Sp{}), "");

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<const int, 4>;

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<int>;
        testRuntimeSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0>(Sp{arr});
        testRuntimeSpan<Sp, 1>(Sp{arr});
        testRuntimeSpan<Sp, 2>(Sp{arr});
        testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
        using Sp = std::span<int, 3>;

        testRuntimeSpan<Sp, 0>(Sp{arr});
        testRuntimeSpan<Sp, 1>(Sp{arr});
        testRuntimeSpan<Sp, 2>(Sp{arr});
        testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
        using Sp = std::span<std::string>;
        testConstexprSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0>(Sp{sarr});
        testRuntimeSpan<Sp, 1>(Sp{sarr});
        testRuntimeSpan<Sp, 2>(Sp{sarr});
        testRuntimeSpan<Sp, 3>(Sp{sarr});
        testRuntimeSpan<Sp, 4>(Sp{sarr});
        testRuntimeSpan<Sp, 5>(Sp{sarr});
    }

    {
        using Sp = std::span<std::string, 5>;

        testRuntimeSpan<Sp, 0>(Sp{sarr});
        testRuntimeSpan<Sp, 1>(Sp{sarr});
        testRuntimeSpan<Sp, 2>(Sp{sarr});
        testRuntimeSpan<Sp, 3>(Sp{sarr});
        testRuntimeSpan<Sp, 4>(Sp{sarr});
        testRuntimeSpan<Sp, 5>(Sp{sarr});
    }
}

namespace
{
    namespace subspan
    {
        template <typename Span, std::size_t Offset, size_t Count>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template subspan<Offset, Count>())));
            LIBCPP_ASSERT((noexcept(sp.subspan(Offset, Count))));
            auto s1 = sp.template subspan<Offset, Count>();
            auto s2 = sp.subspan(Offset, Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count);
            static_assert(S2::extent == std::dynamic_extent, "");
            return
                s1.data() == s2.data()
                && s1.size() == s2.size()
                && std::equal(s1.begin(), s1.end(), sp.begin() + Offset);
        }

        template <typename Span, std::size_t Offset>
        constexpr bool testConstexprSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template subspan<Offset>())));
            LIBCPP_ASSERT((noexcept(sp.subspan(Offset))));
            auto s1 = sp.template subspan<Offset>();
            auto s2 = sp.subspan(Offset);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == (Span::extent == std::dynamic_extent ? std::dynamic_extent : Span::extent - Offset), "");
            static_assert(S2::extent == std::dynamic_extent, "");
            return
                s1.data() == s2.data()
                && s1.size() == s2.size()
                && std::equal(s1.begin(), s1.end(), sp.begin() + Offset, sp.end());
        }


        template <typename Span, std::size_t Offset, size_t Count>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template subspan<Offset, Count>())));
            LIBCPP_ASSERT((noexcept(sp.subspan(Offset, Count))));
            auto s1 = sp.template subspan<Offset, Count>();
            auto s2 = sp.subspan(Offset, Count);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == Count);
            static_assert(S2::extent == std::dynamic_extent, "");
            assert(s1.data() == s2.data());
            assert(s1.size() == s2.size());
            assert(std::equal(s1.begin(), s1.end(), sp.begin() + Offset));
        }


        template <typename Span, std::size_t Offset>
        void testRuntimeSpan(Span sp)
        {
            LIBCPP_ASSERT((noexcept(sp.template subspan<Offset>())));
            LIBCPP_ASSERT((noexcept(sp.subspan(Offset))));
            auto s1 = sp.template subspan<Offset>();
            auto s2 = sp.subspan(Offset);
            using S1 = decltype(s1);
            using S2 = decltype(s2);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
            ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
            static_assert(S1::extent == (Span::extent == std::dynamic_extent ? std::dynamic_extent : Span::extent - Offset), "");
            static_assert(S2::extent == std::dynamic_extent, "");
            assert(s1.data() == s2.data());
            assert(s1.size() == s2.size());
            assert(std::equal(s1.begin(), s1.end(), sp.begin() + Offset, sp.end()));
        }


        constexpr int carr1[] = {1, 2, 3, 4};
        int  arr1[] = {5, 6, 7};

    }
}

TEST(subspan)
{
    using namespace subspan;

    {
        using Sp = std::span<const int>;
        static_assert(testConstexprSpan<Sp, 0>(Sp{}), "");

        static_assert(testConstexprSpan<Sp, 0, 4>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 0>(Sp{carr1}), "");

        static_assert(testConstexprSpan<Sp, 1, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4, 0>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<const int, 4>;

        static_assert(testConstexprSpan<Sp, 0, 4>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 0, 0>(Sp{carr1}), "");

        static_assert(testConstexprSpan<Sp, 1, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4, 0>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<const int>;
        static_assert(testConstexprSpan<Sp, 0>(Sp{}), "");

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<const int, 4>;

        static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");

        static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
        static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
        using Sp = std::span<int>;
        testRuntimeSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0, 3>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 0>(Sp{arr1});

        testRuntimeSpan<Sp, 1, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 2, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 3, 0>(Sp{arr1});
    }

    {
        using Sp = std::span<int, 3>;

        testRuntimeSpan<Sp, 0, 3>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 0, 0>(Sp{arr1});

        testRuntimeSpan<Sp, 1, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 2, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 3, 0>(Sp{arr1});
    }

    {
        using Sp = std::span<int>;
        testRuntimeSpan<Sp, 0>(Sp{});

        testRuntimeSpan<Sp, 0>(Sp{arr1});
        testRuntimeSpan<Sp, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 3>(Sp{arr1});
    }

    {
        using Sp = std::span<int, 3>;

        testRuntimeSpan<Sp, 0>(Sp{arr1});
        testRuntimeSpan<Sp, 1>(Sp{arr1});
        testRuntimeSpan<Sp, 2>(Sp{arr1});
        testRuntimeSpan<Sp, 3>(Sp{arr1});
    }
}



#endif
