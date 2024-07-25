#ifdef TEST_HAS_CONTAINERS_LIST

#include <list>
#include <cassert>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/min_allocator.h"
#include "LLVM.TestSuite/MoveOnly.h"
#include "LLVM.TestSuite/DefaultOnly.h"


TEST(empty)
{
    {
        typedef std::list<int> C;
        C c;
        ASSERT_NOEXCEPT(c.empty());
        assert(c.empty());
        c.push_back(C::value_type(1));
        assert(!c.empty());
        c.clear();
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::list<int, min_allocator<int>> C;
        C c;
        ASSERT_NOEXCEPT(c.empty());
        assert(c.empty());
        c.push_back(C::value_type(1));
        assert(!c.empty());
        c.clear();
        assert(c.empty());
    }
#endif
}

TEST(resize_size)
{
    {
        std::list<int> l(5, 2);
        l.resize(2);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert(l == std::list<int>(2, 2));
    }
    {
        std::list<int> l(5, 2);
        l.resize(10);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 0);
    }
#if TEST_STD_VER >= 11
    {
        std::list<DefaultOnly> l(10);
        l.resize(5);
        assert(l.size() == 5);
        assert(std::distance(l.begin(), l.end()) == 5);
    }
    {
        std::list<DefaultOnly> l(10);
        l.resize(20);
        assert(l.size() == 20);
        assert(std::distance(l.begin(), l.end()) == 20);
    }
    {
        std::list<int, min_allocator<int>> l(5, 2);
        l.resize(2);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert((l == std::list<int, min_allocator<int>>(2, 2)));
    }
    {
        std::list<int, min_allocator<int>> l(5, 2);
        l.resize(10);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 0);
    }
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l(10);
        l.resize(5);
        assert(l.size() == 5);
        assert(std::distance(l.begin(), l.end()) == 5);
    }
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l(10);
        l.resize(20);
        assert(l.size() == 20);
        assert(std::distance(l.begin(), l.end()) == 20);
    }
#endif
}

TEST(resize_size_value)
{
    {
        std::list<double> l(5, 2);
        l.resize(2, 3.5);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert(l == std::list<double>(2, 2));
    }
    {
        std::list<double> l(5, 2);
        l.resize(10, 3.5);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 3.5);
    }
#if TEST_STD_VER >= 11
    {
        std::list<double, min_allocator<double>> l(5, 2);
        l.resize(2, 3.5);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert((l == std::list<double, min_allocator<double>>(2, 2)));
    }
    {
        std::list<double, min_allocator<double>> l(5, 2);
        l.resize(10, 3.5);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 3.5);
    }
#endif
}

TEST(size)
{
    {
        typedef std::list<int> C;
        C c;
        ASSERT_NOEXCEPT(c.size());
        assert(c.size() == 0);
        c.push_back(C::value_type(2));
        assert(c.size() == 1);
        c.push_back(C::value_type(1));
        assert(c.size() == 2);
        c.push_back(C::value_type(3));
        assert(c.size() == 3);
        c.erase(c.begin());
        assert(c.size() == 2);
        c.erase(c.begin());
        assert(c.size() == 1);
        c.erase(c.begin());
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::list<int, min_allocator<int>> C;
        C c;
        ASSERT_NOEXCEPT(c.size());
        assert(c.size() == 0);
        c.push_back(C::value_type(2));
        assert(c.size() == 1);
        c.push_back(C::value_type(1));
        assert(c.size() == 2);
        c.push_back(C::value_type(3));
        assert(c.size() == 3);
        c.erase(c.begin());
        assert(c.size() == 2);
        c.erase(c.begin());
        assert(c.size() == 1);
        c.erase(c.begin());
        assert(c.size() == 0);
    }
#endif
}

TEST(assign_copy)
{
    {
        std::list<int, test_allocator<int> > l(3, 2, test_allocator<int>(5));
        std::list<int, test_allocator<int> > l2(l, test_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<int>(3));
    }
    {
        std::list<int, other_allocator<int> > l(3, 2, other_allocator<int>(5));
        std::list<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(5));
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int> > l(3, 2, min_allocator<int>());
        std::list<int, min_allocator<int> > l2(l, min_allocator<int>());
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
#endif
}

TEST(assign_initializer_list)
{
    {
        std::list<int> d;
        d.assign({3, 4, 5, 6});
        assert(d.size() == 4);
        std::list<int>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
    {
        std::list<int, min_allocator<int>> d;
        d.assign({3, 4, 5, 6});
        assert(d.size() == 4);
        std::list<int, min_allocator<int>>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
}

TEST(assign_move)
{
    {
        std::list<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> >::iterator it = l.begin();
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
    {
        std::list<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(6));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(6));
    }
    {
        std::list<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::list<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, other_allocator<MoveOnly> > l2(other_allocator<MoveOnly>(6));
        std::list<MoveOnly, other_allocator<MoveOnly> >::iterator it = l.begin();
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
    {
        std::list<MoveOnly, min_allocator<MoveOnly> > l(min_allocator<MoveOnly>{});
        std::list<MoveOnly, min_allocator<MoveOnly> > lo(min_allocator<MoveOnly>{});
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, min_allocator<MoveOnly> > l2(min_allocator<MoveOnly>{});
        std::list<MoveOnly, min_allocator<MoveOnly> >::iterator it = l.begin();
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
}

TEST(copy)
{
    {
        std::list<int> l(3, 2);
        std::list<int> l2 = l;
        assert(l2 == l);
    }
    {
        std::list<int, test_allocator<int> > l(3, 2, test_allocator<int>(5));
        std::list<int, test_allocator<int> > l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == l.get_allocator());
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, other_allocator<int> > l(3, 2, other_allocator<int>(5));
        std::list<int, other_allocator<int> > l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(-2));
    }
    {
        std::list<int, min_allocator<int>> l(3, 2);
        std::list<int, min_allocator<int>> l2 = l;
        assert(l2 == l);
    }
    {
        std::list<int, min_allocator<int> > l(3, 2, min_allocator<int>());
        std::list<int, min_allocator<int> > l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == l.get_allocator());
    }
#endif
}

TEST(copy_alloc)
{
    {
        std::list<int, test_allocator<int> > l(3, 2, test_allocator<int>(5));
        std::list<int, test_allocator<int> > l2(l, test_allocator<int>(3));
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<int>(3));
    }
    {
        std::list<int, other_allocator<int> > l(3, 2, other_allocator<int>(5));
        std::list<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(3));
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int> > l(3, 2, min_allocator<int>());
        std::list<int, min_allocator<int> > l2(l, min_allocator<int>());
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
#endif
}

TEST(default)
{
    {
        std::list<int> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<DefaultOnly> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int> l((std::allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, min_allocator<int>> l((min_allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int> l = {};
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, explicit_allocator<int>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, explicit_allocator<int>> l((explicit_allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
#endif
}

TEST(initializer_list)
{
    {
        std::list<int> d = {3, 4, 5, 6};
        assert(d.size() == 4);
        std::list<int>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
    {
        std::list<int, min_allocator<int>> d = {3, 4, 5, 6};
        assert(d.size() == 4);
        std::list<int, min_allocator<int>>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
}

TEST(initializer_list_alloc)
{
    {
        std::list<int, test_allocator<int>> d({3, 4, 5, 6}, test_allocator<int>(3));
        assert(d.get_allocator() == test_allocator<int>(3));
        assert(d.size() == 4);
        std::list<int, test_allocator<int>>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
    {
        std::list<int, min_allocator<int>> d({3, 4, 5, 6}, min_allocator<int>());
        assert(d.get_allocator() == min_allocator<int>());
        assert(d.size() == 4);
        std::list<int, min_allocator<int>>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
}

TEST(move)
{
    {
        std::list<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, test_allocator<MoveOnly> >::iterator it = l.begin();
        std::list<MoveOnly, test_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
    {
        std::list<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::list<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, other_allocator<MoveOnly> >::iterator it = l.begin();
        std::list<MoveOnly, other_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
    {
        std::list<MoveOnly, min_allocator<MoveOnly> > l(min_allocator<MoveOnly>{});
        std::list<MoveOnly, min_allocator<MoveOnly> > lo(min_allocator<MoveOnly>{});
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, min_allocator<MoveOnly> >::iterator it = l.begin();
        std::list<MoveOnly, min_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(it == l2.begin());  // Iterators remain valid
    }
}

TEST(move_alloc)
{
    {
        std::list<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, test_allocator<MoveOnly> > l2(std::move(l), test_allocator<MoveOnly>(6));
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(6));
    }
    {
        std::list<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::list<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, test_allocator<MoveOnly> > l2(std::move(l), test_allocator<MoveOnly>(5));
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(5));
    }
    {
        std::list<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::list<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, other_allocator<MoveOnly> > l2(std::move(l), other_allocator<MoveOnly>(4));
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == other_allocator<MoveOnly>(4));
    }
    {
        std::list<MoveOnly, min_allocator<MoveOnly> > l(min_allocator<MoveOnly>{});
        std::list<MoveOnly, min_allocator<MoveOnly> > lo(min_allocator<MoveOnly>{});
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        std::list<MoveOnly, min_allocator<MoveOnly> > l2(std::move(l), min_allocator<MoveOnly>());
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == min_allocator<MoveOnly>());
    }
}

TEST(op_equal_initializer_list)
{
    {
        std::list<int> d;
        d = {3, 4, 5, 6};
        assert(d.size() == 4);
        std::list<int>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
    {
        std::list<int, min_allocator<int>> d;
        d = {3, 4, 5, 6};
        assert(d.size() == 4);
        std::list<int, min_allocator<int>>::iterator i = d.begin();
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
    }
}

TEST(clear)
{
    {
        int a[] = {1, 2, 3};
        std::list<int> c(a, a + 3);
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        int a[] = {1, 2, 3};
        std::list<int, min_allocator<int>> c(a, a + 3);
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.empty());
    }
#endif
}

namespace
{
    class A
    {
        int i_;
        double d_;

        A(const A&);
        A& operator=(const A&);
    public:
        A(int i, double d)
            : i_(i), d_(d)
        {}

        int geti() const
        {
            return i_;
        }
        double getd() const
        {
            return d_;
        }
    };
}

TEST(emplace)
{
    {
        std::list<A> c;
        c.emplace(c.cbegin(), 2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace(c.cend(), 3, 4.5);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
    {
        std::list<A, min_allocator<A>> c;
        c.emplace(c.cbegin(), 2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace(c.cend(), 3, 4.5);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
}

TEST(emplace_back)
{
    {
        std::list<A> c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
    {
        std::list<A, min_allocator<A>> c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
}

TEST(emplace_front)
{
    {
        std::list<A> c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_front(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.front());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        A& r2 = c.emplace_front(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.front());
    #else
        c.emplace_front(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace_front(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 3);
        assert(c.front().getd() == 4.5);
        assert(c.back().geti() == 2);
        assert(c.back().getd() == 3.5);
    }

    {
        std::list<A, min_allocator<A>> c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_front(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.front());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        A& r2 = c.emplace_front(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.front());
    #else
        c.emplace_front(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        c.emplace_front(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 3);
        assert(c.front().getd() == 4.5);
        assert(c.back().geti() == 2);
        assert(c.back().getd() == 3.5);
    }
}

TEST(erase_iter)
{
    {
        int a1[] = {1, 2, 3};
        std::list<int> l1(a1, a1 + 3);
        std::list<int>::const_iterator i = l1.begin();
        ++i;
        std::list<int>::iterator j = l1.erase(i);
        assert(l1.size() == 2);
        assert(std::distance(l1.begin(), l1.end()) == 2);
        assert(*j == 3);
        assert(*l1.begin() == 1);
        assert(*std::next(l1.begin()) == 3);
        j = l1.erase(j);
        assert(j == l1.end());
        assert(l1.size() == 1);
        assert(std::distance(l1.begin(), l1.end()) == 1);
        assert(*l1.begin() == 1);
        j = l1.erase(l1.begin());
        assert(j == l1.end());
        assert(l1.size() == 0);
        assert(std::distance(l1.begin(), l1.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        int a1[] = {1, 2, 3};
        std::list<int, min_allocator<int>> l1(a1, a1 + 3);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        ++i;
        std::list<int, min_allocator<int>>::iterator j = l1.erase(i);
        assert(l1.size() == 2);
        assert(std::distance(l1.begin(), l1.end()) == 2);
        assert(*j == 3);
        assert(*l1.begin() == 1);
        assert(*std::next(l1.begin()) == 3);
        j = l1.erase(j);
        assert(j == l1.end());
        assert(l1.size() == 1);
        assert(std::distance(l1.begin(), l1.end()) == 1);
        assert(*l1.begin() == 1);
        j = l1.erase(l1.begin());
        assert(j == l1.end());
        assert(l1.size() == 0);
        assert(std::distance(l1.begin(), l1.end()) == 0);
    }
#endif
}

TEST(erase_iter_iter)
{
    int a1[] = {1, 2, 3};
    {
        std::list<int> l1(a1, a1 + 3);
        std::list<int>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(std::distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
    }
    {
        std::list<int> l1(a1, a1 + 3);
        std::list<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(std::distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert(l1 == std::list<int>(a1 + 1, a1 + 3));
    }
    {
        std::list<int> l1(a1, a1 + 3);
        std::list<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(std::distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert(l1 == std::list<int>(a1 + 2, a1 + 3));
    }
    {
        std::list<int> l1(a1, a1 + 3);
        std::list<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(std::distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> l1(a1, a1 + 3);
        std::list<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(std::distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1 + 3);
        std::list<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(std::distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert((l1 == std::list<int, min_allocator<int>>(a1 + 1, a1 + 3)));
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1 + 3);
        std::list<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(std::distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert((l1 == std::list<int, min_allocator<int>>(a1 + 2, a1 + 3)));
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1 + 3);
        std::list<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(std::distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
    }
#endif
}

TEST(insert_iter_initializer_list)
{
    {
        std::list<int> d(10, 1);
        std::list<int>::iterator i = d.insert(std::next(d.cbegin(), 2), {3, 4, 5, 6});
        assert(d.size() == 14);
        assert(i == std::next(d.begin(), 2));
        i = d.begin();
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
    }
    {
        std::list<int, min_allocator<int>> d(10, 1);
        std::list<int, min_allocator<int>>::iterator i = d.insert(std::next(d.cbegin(), 2), {3, 4, 5, 6});
        assert(d.size() == 14);
        assert(i == std::next(d.begin(), 2));
        i = d.begin();
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 3);
        assert(*i++ == 4);
        assert(*i++ == 5);
        assert(*i++ == 6);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
        assert(*i++ == 1);
    }
}

TEST(insert_iter_rvalue)
{
    {
        std::list<MoveOnly> l1;
        l1.insert(l1.cend(), MoveOnly(1));
        assert(l1.size() == 1);
        assert(l1.front() == MoveOnly(1));
        l1.insert(l1.cbegin(), MoveOnly(2));
        assert(l1.size() == 2);
        assert(l1.front() == MoveOnly(2));
        assert(l1.back() == MoveOnly(1));
    }
    {
        std::list<MoveOnly, min_allocator<MoveOnly>> l1;
        l1.insert(l1.cend(), MoveOnly(1));
        assert(l1.size() == 1);
        assert(l1.front() == MoveOnly(1));
        l1.insert(l1.cbegin(), MoveOnly(2));
        assert(l1.size() == 2);
        assert(l1.front() == MoveOnly(2));
        assert(l1.back() == MoveOnly(1));
    }
}

TEST(pop_back)
{
    {
        int a[] = {1, 2, 3};
        std::list<int> c(a, a + 3);
        c.pop_back();
        assert(c == std::list<int>(a, a + 2));
        c.pop_back();
        assert(c == std::list<int>(a, a + 1));
        c.pop_back();
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        int a[] = {1, 2, 3};
        std::list<int, min_allocator<int>> c(a, a + 3);
        c.pop_back();
        assert((c == std::list<int, min_allocator<int>>(a, a + 2)));
        c.pop_back();
        assert((c == std::list<int, min_allocator<int>>(a, a + 1)));
        c.pop_back();
        assert(c.empty());
    }
#endif
}

TEST(pop_front)
{
    {
        int a[] = {1, 2, 3};
        std::list<int> c(a, a + 3);
        c.pop_front();
        assert(c == std::list<int>(a + 1, a + 3));
        c.pop_front();
        assert(c == std::list<int>(a + 2, a + 3));
        c.pop_front();
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        int a[] = {1, 2, 3};
        std::list<int, min_allocator<int>> c(a, a + 3);
        c.pop_front();
        assert((c == std::list<int, min_allocator<int>>(a + 1, a + 3)));
        c.pop_front();
        assert((c == std::list<int, min_allocator<int>>(a + 2, a + 3)));
        c.pop_front();
        assert(c.empty());
    }
#endif
}

TEST(push_back)
{
    {
        std::list<int> c;
        for (int i = 0; i < 5; ++i)
            c.push_back(i);
        int a[] = {0, 1, 2, 3, 4};
        assert(c == std::list<int>(a, a + 5));
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> c;
        for (int i = 0; i < 5; ++i)
            c.push_back(i);
        int a[] = {0, 1, 2, 3, 4};
        assert((c == std::list<int, min_allocator<int>>(a, a + 5)));
    }
#endif
}

TEST(push_front)
{
    {
        std::list<int> c;
        for (int i = 0; i < 5; ++i)
            c.push_front(i);
        int a[] = {4, 3, 2, 1, 0};
        assert(c == std::list<int>(a, a + 5));
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> c;
        for (int i = 0; i < 5; ++i)
            c.push_front(i);
        int a[] = {4, 3, 2, 1, 0};
        assert((c == std::list<int, min_allocator<int>>(a, a + 5)));
    }
#endif
}

TEST(merge)
{
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        int a3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        std::list<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        std::list<int> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        c1.merge(c2);
        assert(c1 == std::list<int>(a3, a3 + sizeof(a3) / sizeof(a3[0])));
        assert(c2.empty());
    }

    {
        int a1[] = {1, 3, 7, 9, 10};
        std::list<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        c1.merge(c1);
        assert((c1 == std::list<int>(a1, a1 + sizeof(a1) / sizeof(a1[0]))));
    }

#if TEST_STD_VER >= 11
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        int a3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        std::list<int, min_allocator<int>> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        std::list<int, min_allocator<int>> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        c1.merge(c2);
        assert((c1 == std::list<int, min_allocator<int>>(a3, a3 + sizeof(a3) / sizeof(a3[0]))));
        assert(c2.empty());
    }
#endif
}

TEST(merge_comp)
{
    {
        int a1[] = {10, 9, 7, 3, 1};
        int a2[] = {11, 8, 6, 5, 4, 2, 0};
        int a3[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        std::list<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        std::list<int> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        c1.merge(c2, std::greater<int>());
        assert(c1 == std::list<int>(a3, a3 + sizeof(a3) / sizeof(a3[0])));
        assert(c2.empty());
    }
    {
        int a1[] = {10, 9, 7, 3, 1};
        std::list<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        c1.merge(c1, std::greater<int>());
        assert((c1 == std::list<int>(a1, a1 + sizeof(a1) / sizeof(a1[0]))));
    }

#if TEST_STD_VER >= 11
    {
        int a1[] = {10, 9, 7, 3, 1};
        int a2[] = {11, 8, 6, 5, 4, 2, 0};
        int a3[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        std::list<int, min_allocator<int>> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        std::list<int, min_allocator<int>> c2(a2, a2 + sizeof(a2) / sizeof(a2[0]));
        c1.merge(c2, std::greater<int>());
        assert((c1 == std::list<int, min_allocator<int>>(a3, a3 + sizeof(a3) / sizeof(a3[0]))));
        assert(c2.empty());
    }
#endif
}

namespace
{
    struct S {
        S(int i) : i_(new int(i))
        {}
        S(const S& rhs) : i_(new int(*rhs.i_))
        {}
        S& operator=(const S& rhs)
        {
            *i_ = *rhs.i_;
            return *this;
        }
        ~S()
        {
            delete i_;
            i_ = NULL;
        }
        bool operator==(const S& rhs) const
        {
            return *i_ == *rhs.i_;
        }
        int get() const
        {
            return *i_;
        }
        int* i_;
    };
}

TEST(remove)
{
    {
        int a1[] = {1, 2, 3, 4};
        int a2[] = {1, 2, 4};
        typedef std::list<int> L;
        L c(a1, a1 + 4);
    #if TEST_STD_VER > 17
        assert(c.remove(3) == 1);
        ASSERT_SAME_TYPE(L::size_type, decltype(c.remove(3)));
    #else
        ASSERT_SAME_TYPE(void, decltype(c.remove(3)));
        c.remove(3);
    #endif

        assert(c == std::list<int>(a2, a2 + 3));
    }
    { // LWG issue #526
        int a1[] = {1, 2, 1, 3, 5, 8, 11};
        int a2[] = {2, 3, 5, 8, 11};
        std::list<int> c(a1, a1 + 7);
        c.remove(c.front());
        assert(c == std::list<int>(a2, a2 + 5));
    }
    {
        int a1[] = {1, 2, 1, 3, 5, 8, 11, 1};
        int a2[] = {2, 3, 5, 8, 11};
        std::list<S> c;
        for (int* ip = a1; ip < a1 + 8; ++ip)
            c.push_back(S(*ip));
    #if TEST_STD_VER > 17
        assert(c.remove(c.front()) == 3);
    #else
        c.remove(c.front());
    #endif
        std::list<S>::const_iterator it = c.begin();
        for (int* ip = a2; ip < a2 + 5; ++ip, ++it) {
            assert(it != c.end());
            assert(*ip == it->get());
        }
        assert(it == c.end());
    }
    {
        typedef no_default_allocator<int> Alloc;
        typedef std::list<int, Alloc> List;
        int a1[] = {1, 2, 3, 4};
        int a2[] = {1, 2, 4};
        List c(a1, a1 + 4, Alloc::create());
    #if TEST_STD_VER > 17
        assert(c.remove(3) == 1);
    #else
        c.remove(3);
    #endif
        assert(c == List(a2, a2 + 3, Alloc::create()));
    }
#if TEST_STD_VER >= 11
    {
        int a1[] = {1, 2, 3, 4};
        int a2[] = {1, 2, 4};
        std::list<int, min_allocator<int>> c(a1, a1 + 4);
    #if TEST_STD_VER > 17
        assert(c.remove(3) == 1);
    #else
        c.remove(3);
    #endif
        assert((c == std::list<int, min_allocator<int>>(a2, a2 + 3)));
    }
#endif
}

TEST(reverse)
{
    {
        int a1[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        int a2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        std::list<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        c1.reverse();
        assert(c1 == std::list<int>(a2, a2 + sizeof(a2) / sizeof(a2[0])));
    }
#if TEST_STD_VER >= 11
    {
        int a1[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        int a2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        std::list<int, min_allocator<int>> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        c1.reverse();
        assert((c1 == std::list<int, min_allocator<int>>(a2, a2 + sizeof(a2) / sizeof(a2[0]))));
    }
#endif
}

TEST(unique)
{
    {
        int a1[] = {2, 1, 1, 4, 4, 4, 4, 3, 3};
        int a2[] = {2, 1, 4, 3};
        typedef std::list<int> L;
        L c(a1, a1 + sizeof(a1) / sizeof(a1[0]));
    #if TEST_STD_VER > 17
        ASSERT_SAME_TYPE(L::size_type, decltype(c.unique()));
        assert(c.unique() == 5);
    #else
        ASSERT_SAME_TYPE(void, decltype(c.unique()));
        c.unique();
    #endif
        assert(c == std::list<int>(a2, a2 + 4));
    }
#if TEST_STD_VER >= 11
    {
        int a1[] = {2, 1, 1, 4, 4, 4, 4, 3, 3};
        int a2[] = {2, 1, 4, 3};
        std::list<int, min_allocator<int>> c(a1, a1 + sizeof(a1) / sizeof(a1[0]));
    #if TEST_STD_VER > 17
        assert(c.unique() == 5);
    #else
        c.unique();
    #endif
        assert((c == std::list<int, min_allocator<int>>(a2, a2 + 4)));
    }
#endif
}


#endif
