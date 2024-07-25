#ifdef TEST_HAS_CONTAINERS_DEQUE

#include <deque>
#include <optional>

#include "LLVM.TestSuite/asan_testing.h"
#include "LLVM.TestSuite/min_allocator.h"
#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/test_comparisons.h"
#include "LLVM.TestSuite/MoveOnly.h"

namespace
{

    class NotConstructible
    {
        NotConstructible(const NotConstructible&);
        NotConstructible& operator=(const NotConstructible&);
    public:
    };

    inline bool operator==(const NotConstructible&, const NotConstructible&)
    {
        return true;
    }

    class Emplaceable
    {
        Emplaceable(const Emplaceable&);
        Emplaceable& operator=(const Emplaceable&);

        int int_;
        double double_;
    public:
        Emplaceable() : int_(0), double_(0)
        {}
        Emplaceable(int i, double d) : int_(i), double_(d)
        {}
        Emplaceable(Emplaceable&& x)
            : int_(x.int_), double_(x.double_)
        {
            x.int_ = 0; x.double_ = 0;
        }
        Emplaceable& operator=(Emplaceable&& x)
        {
            int_ = x.int_; x.int_ = 0;
            double_ = x.double_; x.double_ = 0;
            return *this;
        }

        bool operator==(const Emplaceable& x) const
        {
            return int_ == x.int_ && double_ == x.double_;
        }
        bool operator<(const Emplaceable& x) const
        {
            return int_ < x.int_ || (int_ == x.int_ && double_ < x.double_);
        }

        int get() const
        {
            return int_;
        }
    };

}

namespace std
{
    template <>
    struct hash<NotConstructible>
    {
        typedef NotConstructible argument_type;
        typedef std::size_t result_type;

        std::size_t operator()(const NotConstructible&) const
        {
            return 0;
        }
    };
}

namespace
{
    template <class C>
    C make(int size, int start = 0)
    {
        const int b = 4096 / sizeof(int);
        int init = 0;
        if (start > 0) {
            init = (start + 1) / b + ((start + 1) % b != 0);
            init *= b;
            --init;
        }
        C c(init, 0);
        for (int i = 0; i < init - start; ++i)
            c.pop_back();
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        for (int i = 0; i < size; ++i)
            c.push_back(i);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        for (int i = 0; i < start; ++i)
            c.pop_front();
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        return c;
    }

    template <class C>
    C make_move_only(int size, int start = 0)
    {
        const int b = 4096 / sizeof(int);
        int init = 0;
        if (start > 0) {
            init = (start + 1) / b + ((start + 1) % b != 0);
            init *= b;
            --init;
        }
        C c(init);
        for (int i = 0; i < init - start; ++i)
            c.pop_back();
        for (int i = 0; i < size; ++i)
            c.push_back(MoveOnly(i));
        for (int i = 0; i < start; ++i)
            c.pop_front();
        return c;
    }

    template <class C>
    C make_emplace(int size, int start = 0)
    {
        const int b = 4096 / sizeof(int);
        int init = 0;
        if (start > 0) {
            init = (start + 1) / b + ((start + 1) % b != 0);
            init *= b;
            --init;
        }
        C c(init);
        for (int i = 0; i < init - start; ++i)
            c.pop_back();
        for (int i = 0; i < size; ++i)
            c.push_back(Emplaceable());
        for (int i = 0; i < start; ++i)
            c.pop_front();
        return c;
    }

}

TEST(access)
{
    {
        typedef std::deque<int> C;
        C c = make<std::deque<int> >(10);
        ASSERT_SAME_TYPE(decltype(c[0]), C::reference);
        LIBCPP_ASSERT_NOEXCEPT(c[0]);
        LIBCPP_ASSERT_NOEXCEPT(c.front());
        ASSERT_SAME_TYPE(decltype(c.front()), C::reference);
        LIBCPP_ASSERT_NOEXCEPT(c.back());
        ASSERT_SAME_TYPE(decltype(c.back()), C::reference);
        for (int i = 0; i < 10; ++i)
            assert(c[i] == i);
        for (int i = 0; i < 10; ++i)
            assert(c.at(i) == i);
        assert(c.front() == 0);
        assert(c.back() == 9);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
    {
        typedef std::deque<int> C;
        const C c = make<std::deque<int> >(10);
        ASSERT_SAME_TYPE(decltype(c[0]), C::const_reference);
        LIBCPP_ASSERT_NOEXCEPT(c[0]);
        LIBCPP_ASSERT_NOEXCEPT(c.front());
        ASSERT_SAME_TYPE(decltype(c.front()), C::const_reference);
        LIBCPP_ASSERT_NOEXCEPT(c.back());
        ASSERT_SAME_TYPE(decltype(c.back()), C::const_reference);
        for (int i = 0; i < 10; ++i)
            assert(c[i] == i);
        for (int i = 0; i < 10; ++i)
            assert(c.at(i) == i);
        assert(c.front() == 0);
        assert(c.back() == 9);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::deque<int, min_allocator<int>> C;
        C c = make<std::deque<int, min_allocator<int>> >(10);
        ASSERT_SAME_TYPE(decltype(c[0]), C::reference);
        LIBCPP_ASSERT_NOEXCEPT(c[0]);
        LIBCPP_ASSERT_NOEXCEPT(c.front());
        ASSERT_SAME_TYPE(decltype(c.front()), C::reference);
        LIBCPP_ASSERT_NOEXCEPT(c.back());
        ASSERT_SAME_TYPE(decltype(c.back()), C::reference);
        for (int i = 0; i < 10; ++i)
            assert(c[i] == i);
        for (int i = 0; i < 10; ++i)
            assert(c.at(i) == i);
        assert(c.front() == 0);
        assert(c.back() == 9);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
    {
        typedef std::deque<int, min_allocator<int>> C;
        const C c = make<std::deque<int, min_allocator<int>> >(10);
        ASSERT_SAME_TYPE(decltype(c[0]), C::const_reference);
        LIBCPP_ASSERT_NOEXCEPT(c[0]);
        LIBCPP_ASSERT_NOEXCEPT(c.front());
        ASSERT_SAME_TYPE(decltype(c.front()), C::const_reference);
        LIBCPP_ASSERT_NOEXCEPT(c.back());
        ASSERT_SAME_TYPE(decltype(c.back()), C::const_reference);
        for (int i = 0; i < 10; ++i)
            assert(c[i] == i);
        for (int i = 0; i < 10; ++i)
            assert(c.at(i) == i);
        assert(c.front() == 0);
        assert(c.back() == 9);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#endif
}

TEST(empty)
{
    {
        typedef std::deque<int> C;
        C c;
        ASSERT_NOEXCEPT(c.empty());
        assert(c.empty());
        c.push_back(C::value_type(1));
        assert(!c.empty());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.clear();
        assert(c.empty());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::deque<int, min_allocator<int>> C;
        C c;
        ASSERT_NOEXCEPT(c.empty());
        assert(c.empty());
        c.push_back(C::value_type(1));
        assert(!c.empty());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.clear();
        assert(c.empty());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#endif
}

namespace
{
    template <class C>
    void test_resize_size(C& c1, int size)
    {
        typedef typename C::const_iterator CI;
        typename C::size_type c1_osize = c1.size();
        c1.resize(size);
        assert(c1.size() == static_cast<std::size_t>(size));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        CI i = c1.begin();
        for (int j = 0; static_cast<std::size_t>(j) < std::min(c1_osize, c1.size()); ++j, ++i)
            assert(*i == j);
        for (std::size_t j = c1_osize; j < c1.size(); ++j, ++i)
            assert(*i == 0);
    }

    template <class C>
    void test_resize_size_N(int start, int N, int M)
    {
        C c1 = make<C>(N, start);
        test_resize_size(c1, M);
    }
}

TEST(resize_size)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_N<std::deque<int> >(rng[i], rng[j], rng[k]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_N<std::deque<int, min_allocator<int>>>(rng[i], rng[j], rng[k]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_N<std::deque<int, safe_allocator<int>>>(rng[i], rng[j], rng[k]);
    }
#endif
}

namespace
{
    template <class C>
    void test_resize_size_value(C& c1, int size, int x)
    {
        typedef typename C::const_iterator CI;
        typename C::size_type c1_osize = c1.size();
        c1.resize(size, x);
        assert(c1.size() == static_cast<std::size_t>(size));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        CI i = c1.begin();
        for (int j = 0; static_cast<std::size_t>(j) < std::min(c1_osize, c1.size()); ++j, ++i)
            assert(*i == j);
        for (std::size_t j = c1_osize; j < c1.size(); ++j, ++i)
            assert(*i == x);
    }

    template <class C>
    void test_resize_size_value_N(int start, int N, int M)
    {
        C c1 = make<C>(N, start);
        test_resize_size_value(c1, M, -10);
    }
}

TEST(resize_size_value)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_value_N<std::deque<int> >(rng[i], rng[j], rng[k]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_value_N<std::deque<int, min_allocator<int>>>(rng[i], rng[j], rng[k]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_resize_size_value_N<std::deque<int, safe_allocator<int>>>(rng[i], rng[j], rng[k]);
    }
#endif
}

namespace
{
    template <class C>
    void test_shrink_to_fit(C& c1)
    {
        C s = c1;
        c1.shrink_to_fit();
        assert(c1 == s);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
    }

    template <class C>
    void test_shrink_to_fit_N(int start, int N)
    {
        C c1 = make<C>(N, start);
        test_shrink_to_fit(c1);
    }

}
TEST(shrink_to_fit)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_shrink_to_fit_N<std::deque<int> >(rng[i], rng[j]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_shrink_to_fit_N<std::deque<int, min_allocator<int>> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_shrink_to_fit_N<std::deque<int, safe_allocator<int>> >(rng[i], rng[j]);
    }
#endif
}

TEST(size)
{
    {
        typedef std::deque<int> C;
        C c;
        ASSERT_NOEXCEPT(c.size());
        assert(c.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(2));
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(1));
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(3));
        assert(c.size() == 3);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::deque<int, min_allocator<int>> C;
        C c;
        ASSERT_NOEXCEPT(c.size());
        assert(c.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(2));
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(1));
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.push_back(C::value_type(3));
        assert(c.size() == 3);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.erase(c.begin());
        assert(c.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#endif
}

namespace
{
    template <class T, class Allocator>
    void test_alloc(const Allocator& a)
    {
        std::deque<T, Allocator> d(a);
        assert(d.size() == 0);
        assert(d.get_allocator() == a);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(d));
    }

}

TEST(alloc)
{
    test_alloc<int>(std::allocator<int>());
    test_alloc<NotConstructible>(test_allocator<NotConstructible>(3));
#if TEST_STD_VER >= 11
    test_alloc<int>(min_allocator<int>());
    test_alloc<int>(safe_allocator<int>());
    test_alloc<NotConstructible>(min_allocator<NotConstructible>{});
    test_alloc<NotConstructible>(safe_allocator<NotConstructible>{});
    test_alloc<int>(explicit_allocator<int>());
    test_alloc<NotConstructible>(explicit_allocator<NotConstructible>{});
#endif
}

TEST(assign_initializer_list)
{
    {
        std::deque<int> d;
        d.assign({3, 4, 5, 6});
        assert(d.size() == 4);
        assert(d[0] == 3);
        assert(d[1] == 4);
        assert(d[2] == 5);
        assert(d[3] == 6);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(d));
    }
    {
        std::deque<int, min_allocator<int>> d;
        d.assign({3, 4, 5, 6});
        assert(d.size() == 4);
        assert(d[0] == 3);
        assert(d[1] == 4);
        assert(d[2] == 5);
        assert(d[3] == 6);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(d));
    }
}

namespace
{
    template <class C>
    void test_assign_size_value(C& c1, int size, int v)
    {
        typedef typename C::const_iterator CI;
        c1.assign(size, v);
        assert(c1.size() == static_cast<std::size_t>(size));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        for (CI i = c1.begin(); i != c1.end(); ++i)
            assert(*i == v);
    }

    template <class C>
    void test_assign_size_value_N(int start, int N, int M)
    {
        C c1 = make<C>(N, start);
        test_assign_size_value(c1, M, -10);
    }
}

TEST(assign_size_value)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_assign_size_value_N<std::deque<int> >(rng[i], rng[j], rng[k]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    test_assign_size_value_N<std::deque<int, min_allocator<int>> >(rng[i], rng[j], rng[k]);
    }
#endif
}

namespace
{
    template <class C>
    void test_copy(const C& x)
    {
        C c(x);
        assert(c == x);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(x));
    }

}
TEST(copy)
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy(std::deque<int>(ab, an));
    }
    {
        std::deque<int, test_allocator<int> > v(3, 2, test_allocator<int>(5));
        std::deque<int, test_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v2));
    }
#if TEST_STD_VER >= 11
    {
        std::deque<int, other_allocator<int> > v(3, 2, other_allocator<int>(5));
        std::deque<int, other_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == other_allocator<int>(-2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v2));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy(std::deque<int, min_allocator<int>>(ab, an));
    }
    {
        std::deque<int, min_allocator<int> > v(3, 2, min_allocator<int>());
        std::deque<int, min_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(v2));
    }
#endif
}

namespace
{
    template <class C>
    void test_copy_alloc(const C& x, const typename C::allocator_type& a)
    {
        C c(x, a);
        assert(c == x);
        assert(c.get_allocator() == a);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
}

TEST(copy_alloc)
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy_alloc(std::deque<int, test_allocator<int> >(ab, an, test_allocator<int>(3)),
            test_allocator<int>(4));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy_alloc(std::deque<int, other_allocator<int> >(ab, an, other_allocator<int>(3)),
            other_allocator<int>(4));
    }
#if TEST_STD_VER >= 11
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy_alloc(std::deque<int, min_allocator<int> >(ab, an, min_allocator<int>()),
            min_allocator<int>());
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        test_copy_alloc(std::deque<int, safe_allocator<int> >(ab, an, safe_allocator<int>()),
            safe_allocator<int>());
    }
#endif
}

TEST(move)
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef test_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(1));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(2));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        A old_a = c1.get_allocator();
        std::deque<MoveOnly, A> c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == old_a);
        assert(c1.get_allocator() == A(test_alloc_base::moved_value));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef other_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(1));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(2));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == c1.get_allocator());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef min_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A{});
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A{});
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == c1.get_allocator());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
}

TEST(move_alloc)
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        const int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef test_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(1));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(1));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(std::move(c1), A(3)); // unequal allocator
        assert(c2 == c3);
        assert(c3.get_allocator() == A(3));
        LIBCPP_ASSERT(c1.size() != 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        const int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef test_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(1));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(1));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(std::move(c1), A(1)); // equal allocator
        assert(c2 == c3);
        assert(c3.get_allocator() == A(1));
        LIBCPP_ASSERT(c1.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        const int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef other_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(1));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(1));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(std::move(c1), A(3)); // unequal allocator
        assert(c2 == c3);
        assert(c3.get_allocator() == A(3));
        LIBCPP_ASSERT(c1.size() != 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        const int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef min_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A{});
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A{});
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(std::move(c1), A());  // equal allocator
        assert(c2 == c3);
        assert(c3.get_allocator() == A());
        LIBCPP_ASSERT(c1.size() == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
}

TEST(move_assign)
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef test_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(5));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(5));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(A(5));
        c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == A(5));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef test_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(5));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(5));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(A(6));
        c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() != 0);
        assert(c3.get_allocator() == A(6));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef other_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A(5));
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A(5));
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(A(6));
        c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == A(5));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab) / sizeof(ab[0]);
        typedef min_allocator<MoveOnly> A;
        std::deque<MoveOnly, A> c1(A{});
        for (int* p = ab; p < an; ++p)
            c1.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c2(A{});
        for (int* p = ab; p < an; ++p)
            c2.push_back(MoveOnly(*p));
        std::deque<MoveOnly, A> c3(A{});
        c3 = std::move(c1);
        assert(c2 == c3);
        assert(c1.size() == 0);
        assert(c3.get_allocator() == A());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c2));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c3));
    }
}

namespace
{
    template <class S, class U>
    void test_erase_impl(S s, U val, S expected, std::size_t expected_erased_count)
    {
        ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase(s, val)));
        assert(expected_erased_count == std::erase(s, val));
        assert(s == expected);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(s));
    }

    template <class S>
    void test_erase()
    {
        test_erase_impl(S(), 1, S(), 0);

        test_erase_impl(S({1}), 1, S(), 1);
        test_erase_impl(S({1}), 2, S({1}), 0);

        test_erase_impl(S({1, 2}), 1, S({2}), 1);
        test_erase_impl(S({1, 2}), 2, S({1}), 1);
        test_erase_impl(S({1, 2}), 3, S({1, 2}), 0);
        test_erase_impl(S({1, 1}), 1, S(), 2);
        test_erase_impl(S({1, 1}), 3, S({1, 1}), 0);

        test_erase_impl(S({1, 2, 3}), 1, S({2, 3}), 1);
        test_erase_impl(S({1, 2, 3}), 2, S({1, 3}), 1);
        test_erase_impl(S({1, 2, 3}), 3, S({1, 2}), 1);
        test_erase_impl(S({1, 2, 3}), 4, S({1, 2, 3}), 0);

        test_erase_impl(S({1, 1, 1}), 1, S(), 3);
        test_erase_impl(S({1, 1, 1}), 2, S({1, 1, 1}), 0);
        test_erase_impl(S({1, 1, 2}), 1, S({2}), 2);
        test_erase_impl(S({1, 1, 2}), 2, S({1, 1}), 1);
        test_erase_impl(S({1, 1, 2}), 3, S({1, 1, 2}), 0);
        test_erase_impl(S({1, 2, 2}), 1, S({2, 2}), 1);
        test_erase_impl(S({1, 2, 2}), 2, S({1}), 2);
        test_erase_impl(S({1, 2, 2}), 3, S({1, 2, 2}), 0);

        //  Test cross-type erasure
        using opt = std::optional<typename S::value_type>;
        test_erase_impl(S({1, 2, 1}), opt(), S({1, 2, 1}), 0);
        test_erase_impl(S({1, 2, 1}), opt(1), S({2}), 2);
        test_erase_impl(S({1, 2, 1}), opt(2), S({1, 1}), 1);
        test_erase_impl(S({1, 2, 1}), opt(3), S({1, 2, 1}), 0);
    }

}
TEST(erase)
{
    test_erase<std::deque<int>>();
    test_erase<std::deque<int, min_allocator<int>>>();
    test_erase<std::deque<int, safe_allocator<int>>>();
    test_erase<std::deque<int, test_allocator<int>>>();

    test_erase<std::deque<long>>();
    test_erase<std::deque<double>>();
}

namespace
{
    template <class S, class Pred>
    void test_erase_if_impl(S s, Pred p, S expected, std::size_t expected_erased_count)
    {
        ASSERT_SAME_TYPE(typename S::size_type, decltype(std::erase_if(s, p)));
        assert(expected_erased_count == std::erase_if(s, p));
        assert(s == expected);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(s));
    }

    template <typename S>
    void test_erase_if()
    {
        auto is1 = [](auto v)
        {
            return v == 1;
        };
        auto is2 = [](auto v)
        {
            return v == 2;
        };
        auto is3 = [](auto v)
        {
            return v == 3;
        };
        auto is4 = [](auto v)
        {
            return v == 4;
        };
        auto True = [](auto)
        {
            return true;
        };
        auto False = [](auto)
        {
            return false;
        };

        test_erase_if_impl(S(), is1, S(), 0);

        test_erase_if_impl(S({1}), is1, S(), 1);
        test_erase_if_impl(S({1}), is2, S({1}), 0);

        test_erase_if_impl(S({1, 2}), is1, S({2}), 1);
        test_erase_if_impl(S({1, 2}), is2, S({1}), 1);
        test_erase_if_impl(S({1, 2}), is3, S({1, 2}), 0);
        test_erase_if_impl(S({1, 1}), is1, S(), 2);
        test_erase_if_impl(S({1, 1}), is3, S({1, 1}), 0);

        test_erase_if_impl(S({1, 2, 3}), is1, S({2, 3}), 1);
        test_erase_if_impl(S({1, 2, 3}), is2, S({1, 3}), 1);
        test_erase_if_impl(S({1, 2, 3}), is3, S({1, 2}), 1);
        test_erase_if_impl(S({1, 2, 3}), is4, S({1, 2, 3}), 0);

        test_erase_if_impl(S({1, 1, 1}), is1, S(), 3);
        test_erase_if_impl(S({1, 1, 1}), is2, S({1, 1, 1}), 0);
        test_erase_if_impl(S({1, 1, 2}), is1, S({2}), 2);
        test_erase_if_impl(S({1, 1, 2}), is2, S({1, 1}), 1);
        test_erase_if_impl(S({1, 1, 2}), is3, S({1, 1, 2}), 0);
        test_erase_if_impl(S({1, 2, 2}), is1, S({2, 2}), 1);
        test_erase_if_impl(S({1, 2, 2}), is2, S({1}), 2);
        test_erase_if_impl(S({1, 2, 2}), is3, S({1, 2, 2}), 0);

        test_erase_if_impl(S({1, 2, 3}), True, S(), 3);
        test_erase_if_impl(S({1, 2, 3}), False, S({1, 2, 3}), 0);
    }
}

TEST(erase_if)
{
    test_erase_if<std::deque<int>>();
    test_erase_if<std::deque<int, min_allocator<int>>>();
    test_erase_if<std::deque<int, safe_allocator<int>>>();
    test_erase_if<std::deque<int, test_allocator<int>>>();

    test_erase_if<std::deque<long>>();
    test_erase_if<std::deque<double>>();
}

TEST(clear)
{
    {
        typedef NotConstructible T;
        typedef std::deque<T> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
    {
        typedef int T;
        typedef std::deque<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));

        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
    {
        typedef NotConstructible T;
        typedef std::deque<T, min_allocator<T>> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
    {
        typedef int T;
        typedef std::deque<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));

        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
#endif
}

namespace
{
    template <class C>
    void test_emplace(int P, C& c1)
    {
        typedef typename C::const_iterator CI;
        std::size_t c1_osize = c1.size();
        CI i = c1.emplace(c1.begin() + P, Emplaceable(1, 2.5));
        assert(i == c1.begin() + P);
        assert(c1.size() == c1_osize + 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(*i == Emplaceable(1, 2.5));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
    }

    template <class C>
    void test_emplace_N(int start, int N)
    {
        for (int i = 0; i <= 3; ++i) {
            if (0 <= i && i <= N) {
                C c1 = make_emplace<C>(N, start);
                test_emplace(i, c1);
            }
        }
        for (int i = N / 2 - 1; i <= N / 2 + 1; ++i) {
            if (0 <= i && i <= N) {
                C c1 = make_emplace<C>(N, start);
                test_emplace(i, c1);
            }
        }
        for (int i = N - 3; i <= N; ++i) {
            if (0 <= i && i <= N) {
                C c1 = make_emplace<C>(N, start);
                test_emplace(i, c1);
            }
        }
    }
}

TEST(emplace)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_N<std::deque<Emplaceable> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_N<std::deque<Emplaceable, min_allocator<Emplaceable>> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_N<std::deque<Emplaceable, safe_allocator<Emplaceable>> >(rng[i], rng[j]);
    }
}

namespace
{
    template <class C>
    void test_emplace_back(C& c1)
    {
        typedef typename C::iterator I;
        std::size_t c1_osize = c1.size();
    #if TEST_STD_VER > 14
        typedef typename C::reference Ref;
        Ref ref = c1.emplace_back(Emplaceable(1, 2.5));
    #else
        c1.emplace_back(Emplaceable(1, 2.5));
    #endif
        assert(c1.size() == c1_osize + 1);
        assert(std::distance(c1.begin(), c1.end())
            == static_cast<std::ptrdiff_t>(c1.size()));
        I i = c1.end();
        assert(*--i == Emplaceable(1, 2.5));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
    #if TEST_STD_VER > 14
        assert(&(*i) == &ref);
    #endif
    }

    template <class C>
    void test_emplace_back_N(int start, int N)
    {
        C c1 = make_emplace<C>(N, start);
        test_emplace_back(c1);
    }
}

TEST(emplace_back)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_back_N<std::deque<Emplaceable> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_back_N<std::deque<Emplaceable, min_allocator<Emplaceable>> >(rng[i], rng[j]);
    }
    {
        std::deque<Tag_X, TaggingAllocator<Tag_X>> c;
        c.emplace_back();
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_back(1, 2, 3);
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_front();
        assert(c.size() == 3);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_front(1, 2, 3);
        assert(c.size() == 4);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
}

namespace
{
    template <class C>
    void test_emplace_front(C& c1)
    {
        typedef typename C::iterator I;
        std::size_t c1_osize = c1.size();
    #if TEST_STD_VER > 14
        typedef typename C::reference Ref;
        Ref res_ref = c1.emplace_front(Emplaceable(1, 2.5));
    #else
        c1.emplace_front(Emplaceable(1, 2.5));
    #endif
        assert(c1.size() == c1_osize + 1);
        assert(std::distance(c1.begin(), c1.end())
            == static_cast<std::ptrdiff_t>(c1.size()));
        I i = c1.begin();
        assert(*i == Emplaceable(1, 2.5));
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
    #if TEST_STD_VER > 14
        assert(&res_ref == &(*i));
    #endif
    }

    template <class C>
    void test_emplace_front_N(int start, int N)
    {
        C c1 = make_emplace<C>(N, start);
        test_emplace_front(c1);
    }
}

TEST(emplace_front)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_front_N<std::deque<Emplaceable> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_emplace_front_N<std::deque<Emplaceable, min_allocator<Emplaceable>> >(rng[i], rng[j]);
    }
    {
        std::deque<Tag_X, TaggingAllocator<Tag_X>> c;
        c.emplace_front();
        assert(c.size() == 1);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_front(1, 2, 3);
        assert(c.size() == 2);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_front();
        assert(c.size() == 3);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
        c.emplace_front(1, 2, 3);
        assert(c.size() == 4);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
    }
}

namespace
{
    template <class C>
    void test_pop_back(C& c1)
    {
        typedef typename C::iterator I;
        std::size_t c1_osize = c1.size();
        c1.pop_back();
        assert(c1.size() == c1_osize - 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        I i = c1.begin();
        for (int j = 0; static_cast<std::size_t>(j) < c1.size(); ++j, ++i)
            assert(*i == j);
    }

    template <class C>
    void test_pop_back_N(int start, int N)
    {
        if (N != 0) {
            C c1 = make<C>(N, start);
            test_pop_back(c1);
        }
    }
}

TEST(pop_back)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_pop_back_N<std::deque<int> >(rng[i], rng[j]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_pop_back_N<std::deque<int, min_allocator<int>> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_pop_back_N<std::deque<int, safe_allocator<int>> >(rng[i], rng[j]);
    }
#endif
}

namespace
{
    template <class C>
    void test_pop_front(C& c1)
    {
        typedef typename C::iterator I;
        std::size_t c1_osize = c1.size();
        c1.pop_front();
        assert(c1.size() == c1_osize - 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        I i = c1.begin();
        for (int j = 1; static_cast<std::size_t>(j) < c1.size(); ++j, ++i)
            assert(*i == j);
    }

    template <class C>
    void test_pop_front_N(int start, int N)
    {
        if (N != 0) {
            C c1 = make<C>(N, start);
            test_pop_front(c1);
        }
    }
}

TEST(pop_front)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_pop_front_N<std::deque<int> >(rng[i], rng[j]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_pop_front_N<std::deque<int, min_allocator<int>> >(rng[i], rng[j]);
    }
#endif
}

namespace
{
    template <class C>
    void test_push_back(int size)
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2046, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int j = 0; j < N; ++j) {
            C c = make<C>(size, rng[j]);
            typename C::const_iterator it = c.begin();
            LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c));
            for (int i = 0; i < size; ++i, ++it)
                assert(*it == i);
        }
    }
}

TEST(push_back)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2046, 2047, 2048, 2049, 4094, 4095, 4096};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int j = 0; j < N; ++j)
            test_push_back<std::deque<int> >(rng[j]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2046, 2047, 2048, 2049, 4094, 4095, 4096};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int j = 0; j < N; ++j)
            test_push_back<std::deque<int, min_allocator<int>> >(rng[j]);
    }
#endif
}

namespace
{
    template <class C>
    void test_push_front(C& c1, int x)
    {
        typedef typename C::iterator I;
        std::size_t c1_osize = c1.size();
        c1.push_front(x);
        assert(c1.size() == c1_osize + 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        I i = c1.begin();
        assert(*i == x);
        LIBCPP_ASSERT(is_double_ended_contiguous_container_asan_correct(c1));
        ++i;
        for (int j = 0; static_cast<std::size_t>(j) < c1_osize; ++j, ++i)
            assert(*i == j);
    }

    template <class C>
    void test_push_front_N(int start, int N)
    {
        C c1 = make<C>(N, start);
        test_push_front(c1, -10);
    }
}

TEST(push_front)
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_push_front_N<std::deque<int> >(rng[i], rng[j]);
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_push_front_N<std::deque<int, min_allocator<int>> >(rng[i], rng[j]);
    }
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng) / sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                test_push_front_N<std::deque<int, safe_allocator<int>> >(rng[i], rng[j]);
    }
#endif
}

TEST(compare)
{
    {
        const std::deque<int> d1, d2;
        assert(testComparisons(d1, d2, true, false));
    }
    {
        const std::deque<int> d1(1, 1), d2(1, 1);
        assert(testComparisons(d1, d2, true, false));
    }
    {
        int items[3] = {1, 2, 3};
        const std::deque<int> d1(items, items + 3);
        const std::deque<int> d2(items, items + 3);
        assert(testComparisons(d1, d2, true, false));
    }
    {
        const std::deque<int> d1(1, 1), d2;
        assert(testComparisons(d1, d2, false, false));
    }
    {
        const std::deque<int> d1(1, 1), d2(1, 2);
        assert(testComparisons(d1, d2, false, true));
    }
    {
        int items1[2] = {1, 2};
        int items2[2] = {1, 3};
        const std::deque<int> d1(items1, items1 + 2);
        const std::deque<int> d2(items2, items2 + 2);
        assert(testComparisons(d1, d2, false, true));
    }
    {
        int items1[2] = {2, 2};
        int items2[2] = {1, 3};
        const std::deque<int> d1(items1, items1 + 2);
        const std::deque<int> d2(items2, items2 + 2);
        assert(testComparisons(d1, d2, false, false));
    }
    {
        const std::deque<LessAndEqComp> d1, d2;
        assert(testComparisons(d1, d2, true, false));
    }
    {
        const std::deque<LessAndEqComp> d1(1, LessAndEqComp(1));
        const std::deque<LessAndEqComp> d2(1, LessAndEqComp(1));
        assert(testComparisons(d1, d2, true, false));
    }
    {
        LessAndEqComp items[3] = {LessAndEqComp(1), LessAndEqComp(2), LessAndEqComp(3)};
        const std::deque<LessAndEqComp> d1(items, items + 3);
        const std::deque<LessAndEqComp> d2(items, items + 3);
        assert(testComparisons(d1, d2, true, false));
    }
    {
        const std::deque<LessAndEqComp> d1(1, LessAndEqComp(1));
        const std::deque<LessAndEqComp> d2;
        assert(testComparisons(d1, d2, false, false));
    }
    {
        const std::deque<LessAndEqComp> d1(1, LessAndEqComp(1));
        const std::deque<LessAndEqComp> d2(1, LessAndEqComp(2));
        assert(testComparisons(d1, d2, false, true));
    }
    {
        LessAndEqComp items1[2] = {LessAndEqComp(1), LessAndEqComp(2)};
        LessAndEqComp items2[2] = {LessAndEqComp(1), LessAndEqComp(3)};
        const std::deque<LessAndEqComp> d1(items1, items1 + 2);
        const std::deque<LessAndEqComp> d2(items2, items2 + 2);
        assert(testComparisons(d1, d2, false, true));
    }
    {
        LessAndEqComp items1[2] = {LessAndEqComp(2), LessAndEqComp(2)};
        LessAndEqComp items2[2] = {LessAndEqComp(1), LessAndEqComp(3)};
        const std::deque<LessAndEqComp> d1(items1, items1 + 2);
        const std::deque<LessAndEqComp> d2(items2, items2 + 2);
        assert(testComparisons(d1, d2, false, false));
    }
}


#endif
