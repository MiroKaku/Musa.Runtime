#ifdef TEST_HAS_CONTAINERS_FORWARD_LIST

#include <forward_list>
#include <cassert>
#include <random>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/test_iterators.h"
#include "LLVM.TestSuite/min_allocator.h"
#include "LLVM.TestSuite/MoveOnly.h"
#include "LLVM.TestSuite/DefaultOnly.h"


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

TEST(front)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        assert(c.front() == 0);
        c.front() = 10;
        assert(c.front() == 10);
        assert(*c.begin() == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const C c(std::begin(t), std::end(t));
        assert(c.front() == 0);
        assert(*c.begin() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        assert(c.front() == 0);
        c.front() = 10;
        assert(c.front() == 10);
        assert(*c.begin() == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const C c(std::begin(t), std::end(t));
        assert(c.front() == 0);
        assert(*c.begin() == 0);
    }
#endif
}

TEST(alloc)
{
    {
        typedef test_allocator<NotConstructible> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        C c(A(12));
        assert(c.get_allocator() == A(12));
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef min_allocator<NotConstructible> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        C c(A{});
        assert(c.get_allocator() == A());
        assert(c.empty());
    }
    {
        typedef explicit_allocator<NotConstructible> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        C c(A{});
        assert(c.get_allocator() == A());
        assert(c.empty());
    }
#endif
}

TEST(assign_copy)
{
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(10));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(11));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(11));
    }
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(10));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(11));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(11));
    }

    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(10));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(11));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(10));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t0), std::end(t0), A(10));
        C c1(std::begin(t1), std::end(t1), A(11));
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A(10));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef min_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c0(std::begin(t0), std::end(t0), A());
        C c1(std::begin(t1), std::end(t1), A());
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A());
    }
    {
        typedef int T;
        typedef min_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t0), std::end(t0), A());
        C c1(std::begin(t1), std::end(t1), A());
        c1 = c0;
        assert(c1 == c0);
        assert(c1.get_allocator() == A());
    }
#endif
}

TEST(assign_init)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c.assign({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c.assign({10, 11, 12, 13});
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c.assign({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c.assign({10, 11, 12, 13});
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
}

TEST(assign_move)
{
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        T t1[] = {10, 11, 12, 13};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(10));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        T t1[] = {10, 11, 12, 13};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(11));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c1.get_allocator() == A(11));
        assert(!c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {10, 11, 12, 13};
        T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(10));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {10, 11, 12, 13};
        T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(11));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
        assert(c1.get_allocator() == A(11));
        assert(!c0.empty());
    }

    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        T t1[] = {10, 11, 12, 13};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(10));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        T t1[] = {10, 11, 12, 13};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(11));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {10, 11, 12, 13};
        T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(10));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {10, 11, 12, 13};
        T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A(10));
        C c1(I(std::begin(t1)), I(std::end(t1)), A(11));
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
        assert(c1.get_allocator() == A(10));
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        T t1[] = {10, 11, 12, 13};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A());
        C c1(I(std::begin(t1)), I(std::end(t1)), A());
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c1.get_allocator() == A());
        assert(c0.empty());
    }
    {
        typedef MoveOnly T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t0[] = {10, 11, 12, 13};
        T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t0)), I(std::end(t0)), A());
        C c1(I(std::begin(t1)), I(std::end(t1)), A());
        c1 = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c1.cbegin(); i != c1.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
        assert(c1.get_allocator() == A());
        assert(c0.empty());
    }
}

TEST(assign_op_init)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c = {10, 11, 12, 13};
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c = {10, 11, 12, 13};
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
}

TEST(assign_range)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        typedef cpp17_input_iterator<const T*> I;
        c.assign(I(std::begin(t0)), I(std::end(t0)));
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        typedef cpp17_input_iterator<const T*> I;
        c.assign(I(std::begin(t0)), I(std::end(t0)));
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t0[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        typedef cpp17_input_iterator<const T*> I;
        c.assign(I(std::begin(t0)), I(std::end(t0)));
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t0[] = {10, 11, 12, 13};
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        typedef cpp17_input_iterator<const T*> I;
        c.assign(I(std::begin(t0)), I(std::end(t0)));
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, (void) ++n)
            assert(*i == 10 + n);
        assert(n == 4);
    }
#endif
}

TEST(assign_size_value)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c.assign(10, 1);
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 1);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c.assign(4, 10);
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10);
        assert(n == 4);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {10, 11, 12, 13};
        C c(std::begin(t1), std::end(t1));
        c.assign(10, 1);
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 1);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t1), std::end(t1));
        c.assign(4, 10);
        int n = 0;
        for (C::const_iterator i = c.cbegin(); i != c.cend(); ++i, ++n)
            assert(*i == 10);
        assert(n == 4);
    }
#endif
}

TEST(copy)
{
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c = c0;
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(10));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c = c0;
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(-2));
    }
    {
        typedef int T;
        typedef min_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A());
        C c = c0;
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A());
    }
#endif
}

TEST(copy_alloc)
{
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c(c0, A(9));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(9));
    }
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c(c0, A(9));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(9));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef min_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A());
        C c(c0, A());
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A());
    }
#endif
}

TEST(default)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c;
        assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        assert(c.empty());
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c = {};
        assert(c.empty());
    }
#endif
}

TEST(init)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == 10);
    }
}

TEST(init_alloc)
{
    {
        typedef int T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        C c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, A(14));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c.get_allocator() == A(14));
    }
    {
        typedef int T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        C c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, A());
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == 10);
        assert(c.get_allocator() == A());
    }
}

TEST(move)
{
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A(10));
    }
    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A(10));
    }
    {
        typedef MoveOnly T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A());
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A());
    }
}

TEST(move_alloc)
{
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c(std::move(c0), A(10));
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == static_cast<unsigned>(std::end(t) - std::begin(t)));
        assert(c0.empty());
        assert(c.get_allocator() == A(10));
    }
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c(std::move(c0), A(9));
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == static_cast<unsigned>(std::end(t) - std::begin(t)));
        assert(!c0.empty());
        assert(c.get_allocator() == A(9));
    }
    {
        typedef MoveOnly T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A());
        C c(std::move(c0), A());
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, (void) ++n)
            assert(*i == n);
        assert(n == static_cast<unsigned>(std::end(t) - std::begin(t)));
        assert(c0.empty());
        assert(c.get_allocator() == A());
    }
}

TEST(range)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef cpp17_input_iterator<const T*> I;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(I(std::begin(t)), I(std::end(t)));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef cpp17_input_iterator<const T*> I;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(I(std::begin(t)), I(std::end(t)));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
    }
#endif
}

TEST(range_alloc)
{
    {
        typedef int T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        typedef cpp17_input_iterator<const T*> I;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(I(std::begin(t)), I(std::end(t)), A(13));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c.get_allocator() == A(13));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        typedef cpp17_input_iterator<const T*> I;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(I(std::begin(t)), I(std::end(t)), A());
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c.get_allocator() == A());
    }
#endif
}

TEST(size_value)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        T v(6);
        unsigned N = 10;
        C c(N, v);
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == v);
        assert(n == N);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        T v(6);
        unsigned N = 10;
        C c(N, v);
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == v);
        assert(n == N);
    }
#endif
}

TEST(size_value_alloc)
{
    {
        typedef test_allocator<int> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        T v(6);
        unsigned N = 10;
        C c(N, v, A(12));
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == v);
        assert(n == N);
        assert(c.get_allocator() == A(12));
    }
#if TEST_STD_VER >= 11
    {
        typedef min_allocator<int> A;
        typedef A::value_type T;
        typedef std::forward_list<T, A> C;
        T v(6);
        unsigned N = 10;
        C c(N, v, A());
        unsigned n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == v);
        assert(n == N);
        assert(c.get_allocator() == A());
    }
#endif
}

TEST(clear)
{
    {
        typedef NotConstructible T;
        typedef std::forward_list<T> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);

        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef NotConstructible T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);

        c.clear();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#endif
}

TEST(emplace_after)
{
    {
        typedef Emplaceable T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.emplace_after(c.cbefore_begin());
        assert(i == c.begin());
        assert(c.front() == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.emplace_after(c.cbegin(), 1, 2.5);
        assert(i == std::next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin()) == Emplaceable(1, 2.5));
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.emplace_after(std::next(c.cbegin()), 2, 3.5);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin()) == Emplaceable(1, 2.5));
        assert(*std::next(c.begin(), 2) == Emplaceable(2, 3.5));
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.emplace_after(c.cbegin(), 3, 4.5);
        assert(i == std::next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin(), 1) == Emplaceable(3, 4.5));
        assert(*std::next(c.begin(), 2) == Emplaceable(1, 2.5));
        assert(*std::next(c.begin(), 3) == Emplaceable(2, 3.5));
        assert(std::distance(c.begin(), c.end()) == 4);
    }
    {
        typedef Emplaceable T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.emplace_after(c.cbefore_begin());
        assert(i == c.begin());
        assert(c.front() == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.emplace_after(c.cbegin(), 1, 2.5);
        assert(i == std::next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin()) == Emplaceable(1, 2.5));
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.emplace_after(std::next(c.cbegin()), 2, 3.5);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin()) == Emplaceable(1, 2.5));
        assert(*std::next(c.begin(), 2) == Emplaceable(2, 3.5));
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.emplace_after(c.cbegin(), 3, 4.5);
        assert(i == std::next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*std::next(c.begin(), 1) == Emplaceable(3, 4.5));
        assert(*std::next(c.begin(), 2) == Emplaceable(1, 2.5));
        assert(*std::next(c.begin(), 3) == Emplaceable(2, 3.5));
        assert(std::distance(c.begin(), c.end()) == 4);
    }
}

TEST(emplace_front)
{
    {
        typedef Emplaceable T;
        typedef std::forward_list<T> C;
        C c;
    #if TEST_STD_VER > 14
        T& r1 = c.emplace_front();
        assert(c.front() == Emplaceable());
        assert(&r1 == &c.front());
        assert(std::distance(c.begin(), c.end()) == 1);
        T& r2 = c.emplace_front(1, 2.5);
        assert(c.front() == Emplaceable(1, 2.5));
        assert(&r2 == &c.front());
    #else
        c.emplace_front();
        assert(c.front() == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 1);
        c.emplace_front(1, 2.5);
        assert(c.front() == Emplaceable(1, 2.5));
    #endif
        assert(*std::next(c.begin()) == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 2);
    }
    {
        typedef Emplaceable T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
    #if TEST_STD_VER > 14
        T& r1 = c.emplace_front();
        assert(c.front() == Emplaceable());
        assert(&r1 == &c.front());
        assert(std::distance(c.begin(), c.end()) == 1);
        T& r2 = c.emplace_front(1, 2.5);
        assert(c.front() == Emplaceable(1, 2.5));
        assert(&r2 == &c.front());
    #else
        c.emplace_front();
        assert(c.front() == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 1);
        c.emplace_front(1, 2.5);
        assert(c.front() == Emplaceable(1, 2.5));
    #endif
        assert(*std::next(c.begin()) == Emplaceable());
        assert(std::distance(c.begin(), c.end()) == 2);
    }
}

TEST(erase_after_many)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(std::next(c.cbefore_begin(), 4), std::next(c.cbefore_begin(), 4));
        assert(i == std::next(c.cbefore_begin(), 4));
        assert(std::distance(c.begin(), c.end()) == 10);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);
        assert(*std::next(c.begin(), 4) == 4);
        assert(*std::next(c.begin(), 5) == 5);
        assert(*std::next(c.begin(), 6) == 6);
        assert(*std::next(c.begin(), 7) == 7);
        assert(*std::next(c.begin(), 8) == 8);
        assert(*std::next(c.begin(), 9) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 2), std::next(c.cbefore_begin(), 5));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 8);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);
        assert(*std::next(c.begin(), 5) == 7);
        assert(*std::next(c.begin(), 6) == 8);
        assert(*std::next(c.begin(), 7) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 2), std::next(c.cbefore_begin(), 3));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 8);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);
        assert(*std::next(c.begin(), 5) == 7);
        assert(*std::next(c.begin(), 6) == 8);
        assert(*std::next(c.begin(), 7) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 5), std::next(c.cbefore_begin(), 9));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);

        i = c.erase_after(std::next(c.cbefore_begin(), 0), std::next(c.cbefore_begin(), 2));
        assert(i == c.begin());
        assert(std::distance(c.begin(), c.end()) == 4);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 4);
        assert(*std::next(c.begin(), 2) == 5);
        assert(*std::next(c.begin(), 3) == 6);

        i = c.erase_after(std::next(c.cbefore_begin(), 0), std::next(c.cbefore_begin(), 5));
        assert(i == c.begin());
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(std::next(c.cbefore_begin(), 4), std::next(c.cbefore_begin(), 4));
        assert(i == std::next(c.cbefore_begin(), 4));
        assert(std::distance(c.begin(), c.end()) == 10);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);
        assert(*std::next(c.begin(), 4) == 4);
        assert(*std::next(c.begin(), 5) == 5);
        assert(*std::next(c.begin(), 6) == 6);
        assert(*std::next(c.begin(), 7) == 7);
        assert(*std::next(c.begin(), 8) == 8);
        assert(*std::next(c.begin(), 9) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 2), std::next(c.cbefore_begin(), 5));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 8);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);
        assert(*std::next(c.begin(), 5) == 7);
        assert(*std::next(c.begin(), 6) == 8);
        assert(*std::next(c.begin(), 7) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 2), std::next(c.cbefore_begin(), 3));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 8);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);
        assert(*std::next(c.begin(), 5) == 7);
        assert(*std::next(c.begin(), 6) == 8);
        assert(*std::next(c.begin(), 7) == 9);

        i = c.erase_after(std::next(c.cbefore_begin(), 5), std::next(c.cbefore_begin(), 9));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 5);
        assert(*std::next(c.begin(), 4) == 6);

        i = c.erase_after(std::next(c.cbefore_begin(), 0), std::next(c.cbefore_begin(), 2));
        assert(i == c.begin());
        assert(std::distance(c.begin(), c.end()) == 4);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 4);
        assert(*std::next(c.begin(), 2) == 5);
        assert(*std::next(c.begin(), 3) == 6);

        i = c.erase_after(std::next(c.cbefore_begin(), 0), std::next(c.cbefore_begin(), 5));
        assert(i == c.begin());
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#endif
}

TEST(erase_after_one)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(std::next(c.cbefore_begin(), 4));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 4);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 0));
        assert(i == c.begin());
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 2);
        assert(*std::next(c.begin(), 2) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 1));
        assert(i == std::next(c.begin()));
        assert(std::distance(c.begin(), c.end()) == 2);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 1));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(*std::next(c.begin(), 0) == 1);

        i = c.erase_after(std::next(c.cbefore_begin(), 0));
        assert(i == c.begin());
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(std::next(c.cbefore_begin(), 4));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 4);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 0));
        assert(i == c.begin());
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 2);
        assert(*std::next(c.begin(), 2) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 1));
        assert(i == std::next(c.begin()));
        assert(std::distance(c.begin(), c.end()) == 2);
        assert(*std::next(c.begin(), 0) == 1);
        assert(*std::next(c.begin(), 1) == 3);

        i = c.erase_after(std::next(c.cbefore_begin(), 1));
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(*std::next(c.begin(), 0) == 1);

        i = c.erase_after(std::next(c.cbefore_begin(), 0));
        assert(i == c.begin());
        assert(i == c.end());
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#endif
}

TEST(insert_after_const)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0);
        assert(i == c.begin());
        assert(c.front() == 0);
        assert(c.front() == 0);
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.insert_after(c.cbegin(), 1);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.insert_after(std::next(c.cbegin()), 2);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.insert_after(c.cbegin(), 3);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 1);
        assert(*std::next(c.begin(), 3) == 2);
        assert(std::distance(c.begin(), c.end()) == 4);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0);
        assert(i == c.begin());
        assert(c.front() == 0);
        assert(c.front() == 0);
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.insert_after(c.cbegin(), 1);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.insert_after(std::next(c.cbegin()), 2);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.insert_after(c.cbegin(), 3);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 1);
        assert(*std::next(c.begin(), 3) == 2);
        assert(std::distance(c.begin(), c.end()) == 4);
    }
#endif
}

TEST(insert_after_init)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), {});
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), {0, 1, 2});
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), {3, 4});
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 1);
        assert(*std::next(c.begin(), 4) == 2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), {});
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), {0, 1, 2});
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), {3, 4});
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 1);
        assert(*std::next(c.begin(), 4) == 2);
    }
}

TEST(insert_after_range)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        typedef cpp17_input_iterator<const T*> J;
        C c;
        const T t[] = {0, 1, 2, 3, 4};
        I i = c.insert_after(c.cbefore_begin(), J(t), J(t));
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), J(t), J(t + 3));
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), J(t + 3), J(t + 5));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 1);
        assert(*std::next(c.begin(), 4) == 2);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        typedef cpp17_input_iterator<const T*> J;
        C c;
        const T t[] = {0, 1, 2, 3, 4};
        I i = c.insert_after(c.cbefore_begin(), J(t), J(t));
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), J(t), J(t + 3));
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), J(t + 3), J(t + 5));
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 4);
        assert(*std::next(c.begin(), 3) == 1);
        assert(*std::next(c.begin(), 4) == 2);
    }
#endif
}

TEST(insert_after_rv)
{
    {
        typedef MoveOnly T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0);
        assert(i == c.begin());
        assert(c.front() == 0);
        assert(c.front() == 0);
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.insert_after(c.cbegin(), 1);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.insert_after(std::next(c.cbegin()), 2);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.insert_after(c.cbegin(), 3);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 1);
        assert(*std::next(c.begin(), 3) == 2);
        assert(std::distance(c.begin(), c.end()) == 4);
    }
    {
        typedef MoveOnly T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0);
        assert(i == c.begin());
        assert(c.front() == 0);
        assert(c.front() == 0);
        assert(std::distance(c.begin(), c.end()) == 1);

        i = c.insert_after(c.cbegin(), 1);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);

        i = c.insert_after(std::next(c.cbegin()), 2);
        assert(i == std::next(c.begin(), 2));
        assert(c.front() == 0);
        assert(*std::next(c.begin()) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(std::distance(c.begin(), c.end()) == 3);

        i = c.insert_after(c.cbegin(), 3);
        assert(i == std::next(c.begin()));
        assert(c.front() == 0);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 1);
        assert(*std::next(c.begin(), 3) == 2);
        assert(std::distance(c.begin(), c.end()) == 4);
    }
}

TEST(insert_after_size_value)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0, 0);
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), 3, 3);
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 3);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 3);

        i = c.insert_after(c.begin(), 2, 2);
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 3);
        assert(*std::next(c.begin(), 1) == 2);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);
        assert(*std::next(c.begin(), 4) == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), 0, 0);
        assert(i == c.before_begin());
        assert(std::distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), 3, 3);
        assert(i == std::next(c.before_begin(), 3));
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 3);
        assert(*std::next(c.begin(), 1) == 3);
        assert(*std::next(c.begin(), 2) == 3);

        i = c.insert_after(c.begin(), 2, 2);
        assert(i == std::next(c.begin(), 2));
        assert(std::distance(c.begin(), c.end()) == 5);
        assert(*std::next(c.begin(), 0) == 3);
        assert(*std::next(c.begin(), 1) == 2);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 3);
        assert(*std::next(c.begin(), 4) == 3);
    }
#endif
}

TEST(pop_front)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef std::forward_list<T> C;
        C c;
        c.push_front(1);
        c.push_front(3);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(c.front() == 1);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef MoveOnly T;
        typedef std::forward_list<T> C;
        C c;
        c.push_front(1);
        c.push_front(3);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(c.front() == 1);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.push_front(1);
        c.push_front(3);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(c.front() == 1);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef MoveOnly T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.push_front(1);
        c.push_front(3);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 1);
        assert(c.front() == 1);
        c.pop_front();
        assert(std::distance(c.begin(), c.end()) == 0);
    }
#endif
}

TEST(push_front_const)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(std::distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(std::distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);
    }
#endif
}

TEST(push_front_rv)
{
    {
        typedef MoveOnly T;
        typedef std::forward_list<T> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(std::distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);
    }
    {
        typedef MoveOnly T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(std::distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*std::next(c.begin()) == 1);
        assert(std::distance(c.begin(), c.end()) == 2);
    }
}

TEST(resize_size)
{
    {
        typedef DefaultOnly T;
        typedef std::forward_list<T> C;
        C c;
        c.resize(0);
        assert(std::distance(c.begin(), c.end()) == 0);
        c.resize(10);
        assert(std::distance(c.begin(), c.end()) == 10);
        c.resize(20);
        assert(std::distance(c.begin(), c.end()) == 20);
        c.resize(5);
        assert(std::distance(c.begin(), c.end()) == 5);
        c.resize(0);
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        c.resize(3);
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        c.resize(6);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 0);
        assert(*std::next(c.begin(), 4) == 0);
        assert(*std::next(c.begin(), 5) == 0);

        c.resize(6);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 0);
        assert(*std::next(c.begin(), 4) == 0);
        assert(*std::next(c.begin(), 5) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef DefaultOnly T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.resize(0);
        assert(std::distance(c.begin(), c.end()) == 0);
        c.resize(10);
        assert(std::distance(c.begin(), c.end()) == 10);
        c.resize(20);
        assert(std::distance(c.begin(), c.end()) == 20);
        c.resize(5);
        assert(std::distance(c.begin(), c.end()) == 5);
        c.resize(0);
        assert(std::distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        c.resize(3);
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        c.resize(6);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 0);
        assert(*std::next(c.begin(), 4) == 0);
        assert(*std::next(c.begin(), 5) == 0);

        c.resize(6);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 0);
        assert(*std::next(c.begin(), 4) == 0);
        assert(*std::next(c.begin(), 5) == 0);
    }
#endif
}

TEST(resize_size_value)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        c.resize(3, 10);
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        c.resize(6, 10);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 10);
        assert(*std::next(c.begin(), 4) == 10);
        assert(*std::next(c.begin(), 5) == 10);

        c.resize(6, 12);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 10);
        assert(*std::next(c.begin(), 4) == 10);
        assert(*std::next(c.begin(), 5) == 10);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        c.resize(3, 10);
        assert(std::distance(c.begin(), c.end()) == 3);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);

        c.resize(6, 10);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 10);
        assert(*std::next(c.begin(), 4) == 10);
        assert(*std::next(c.begin(), 5) == 10);

        c.resize(6, 12);
        assert(std::distance(c.begin(), c.end()) == 6);
        assert(*std::next(c.begin(), 0) == 0);
        assert(*std::next(c.begin(), 1) == 1);
        assert(*std::next(c.begin(), 2) == 2);
        assert(*std::next(c.begin(), 3) == 10);
        assert(*std::next(c.begin(), 4) == 10);
        assert(*std::next(c.begin(), 5) == 10);
    }
#endif
}

namespace
{
    struct value {
        int a;
        int b;

        friend bool operator<(const value& lhs, const value& rhs)
        {
            return lhs.a < rhs.a;
        }
        friend bool operator==(const value& lhs, const value& rhs)
        {
            return lhs.a == rhs.a && lhs.b == rhs.b;
        }
    };
}

TEST(merge_lvalue)
{
    { // Basic merge operation.
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {3, 5, 6, 7, 12, 13};
        const T t2[] = {0, 1, 2, 4, 8, 9, 10, 11, 14, 15};
        const T t3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(c2);
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
    { // Pointers, references, and iterators should remain valid after merging.
        typedef int T;
        typedef std::forward_list<T> C;
        typedef T* P;
        typedef typename C::iterator I;
        const T to[3] = {0, 1, 2};

        C c2(std::begin(to), std::end(to));
        I io[3] = {c2.begin(), ++c2.begin(), ++ ++c2.begin()};
    #if TEST_STD_VER >= 11
        std::reference_wrapper<T> ro[3] = {*io[0], *io[1], *io[2]};
    #endif
        P po[3] = {&*io[0], &*io[1], &*io[2]};

        C c1;
        c1.merge(c2);
        assert(c2.empty());

        for (std::size_t i = 0; i < 3; ++i) {
            assert(to[i] == *io[i]);
        #if TEST_STD_VER >= 11
            assert(to[i] == ro[i].get());
        #endif
            assert(to[i] == *po[i]);
        }
    }
    { // Sorting is stable.
        typedef value T;
        typedef std::forward_list<T> C;
        const T t1[] = {{0, 0}, {2, 0}, {3, 0}};
        const T t2[] = {{0, 1}, {1, 1}, {2, 1}, {4, 1}};
        const T t3[] = {{0, 0}, {0, 1}, {1, 1}, {2, 0}, {2, 1}, {3, 0}, {4, 1}};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(c2);
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
#if TEST_STD_VER >= 11
    { // Test with a different allocator.
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {3, 5, 6, 7, 12, 13};
        const T t2[] = {0, 1, 2, 4, 8, 9, 10, 11, 14, 15};
        const T t3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(c2);
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
#endif
}

TEST(merge_rvalue)
{
    { // Basic merge operation.
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {3, 5, 6, 7, 12, 13};
        const T t2[] = {0, 1, 2, 4, 8, 9, 10, 11, 14, 15};
        const T t3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(std::move(c2));
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
    { // Pointers, references, and iterators should remain valid after merging.
        typedef int T;
        typedef std::forward_list<T> C;
        typedef T* P;
        typedef typename C::iterator I;
        const T to[3] = {0, 1, 2};

        C c2(std::begin(to), std::end(to));
        I io[3] = {c2.begin(), ++c2.begin(), ++ ++c2.begin()};
        std::reference_wrapper<T> ro[3] = {*io[0], *io[1], *io[2]};
        P po[3] = {&*io[0], &*io[1], &*io[2]};

        C c1;
        c1.merge(std::move(c2));
        assert(c2.empty());

        for (std::size_t i = 0; i < 3; ++i) {
            assert(to[i] == *io[i]);
            assert(to[i] == ro[i].get());
            assert(to[i] == *po[i]);
        }
    }
    { // Sorting is stable.
        typedef value T;
        typedef std::forward_list<T> C;
        const T t1[] = {{0, 0}, {2, 0}, {3, 0}};
        const T t2[] = {{0, 1}, {1, 1}, {2, 1}, {4, 1}};
        const T t3[] = {{0, 0}, {0, 1}, {1, 1}, {2, 0}, {2, 1}, {3, 0}, {4, 1}};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(std::move(c2));
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
    { // Test with a different allocator.
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {3, 5, 6, 7, 12, 13};
        const T t2[] = {0, 1, 2, 4, 8, 9, 10, 11, 14, 15};
        const T t3[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        c1.merge(std::move(c2));
        assert(c2.empty());

        C c3(std::begin(t3), std::end(t3));
        assert(c1 == c3);
    }
}

namespace
{
    template <class L>
    void do_remove(L& l, const typename L::value_type& value, typename L::size_type expected)
    {
        typename L::size_type old_size = std::distance(l.begin(), l.end());
    #if TEST_STD_VER > 17
        ASSERT_SAME_TYPE(decltype(l.remove(value)), typename L::size_type);
        assert(l.remove(value) == expected);
    #else
        ASSERT_SAME_TYPE(decltype(l.remove(value)), void);
        l.remove(value);
    #endif
        assert(old_size - std::distance(l.begin(), l.end()) == expected);
    }

    struct S {
        S(int i) : i_(new int(i))
        {}
        S(const S& rhs) : i_(new int(*rhs.i_))
        {}
        S& operator = (const S& rhs)
        {
            *i_ = *rhs.i_; return *this;
        }
        ~S()
        {
            delete i_; i_ = NULL;
        }
        bool operator == (const S& rhs) const
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
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 4);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 0, 0, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2;
        do_remove(c1, 0, 4);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c1;
        C c2;
        do_remove(c1, 0, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 1);
        assert(c1 == c2);
    }
    {  // LWG issue #526
        typedef int T;
        typedef std::forward_list<T> C;
        int t1[] = {1, 2, 1, 3, 5, 8, 11};
        int t2[] = {2, 3, 5, 8, 11};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, c1.front(), 2);
        assert(c1 == c2);
    }
    {
        typedef S T;
        typedef std::forward_list<T> C;
        int t1[] = {1, 2, 1, 3, 5, 8, 11, 1};
        int t2[] = {2, 3, 5, 8, 11};
        C c;
        for (int* ip = std::end(t1); ip != std::begin(t1);)
            c.push_front(S(*--ip));
        do_remove(c, c.front(), 3);
        C::const_iterator it = c.begin();
        for (int* ip = std::begin(t2); ip != std::end(t2); ++ip, ++it) {
            assert(it != c.end());
            assert(*ip == it->get());
        }
        assert(it == c.end());
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 4);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 0, 0, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2;
        do_remove(c1, 0, 4);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c1;
        C c2;
        do_remove(c1, 0, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_remove(c1, 0, 1);
        assert(c1 == c2);
    }
#endif
}

namespace
{
    template <class L, class Predicate>
    void do_remove_if(L& l, Predicate pred, typename L::size_type expected)
    {
        typename L::size_type old_size = std::distance(l.begin(), l.end());
    #if TEST_STD_VER > 17
        ASSERT_SAME_TYPE(decltype(l.remove_if(pred)), typename L::size_type);
        assert(l.remove_if(pred) == expected);
    #else
        ASSERT_SAME_TYPE(decltype(l.remove_if(pred)), void);
        l.remove_if(pred);
    #endif
        assert(old_size - std::distance(l.begin(), l.end()) == expected);
    }

    bool g(int i)
    {
        return i < 3;
    }

    struct PredLWG526 {
        PredLWG526(int i) : i_(i)
        {}
        ~PredLWG526()
        {
            i_ = -32767;
        }
        bool operator()(const PredLWG526& p) const
        {
            return p.i_ == i_;
        }

        bool operator==(int i) const
        {
            return i == i_;
        }
        int i_;
    };
}

namespace
{
    template <class C>
    void test_reverse(int N)
    {
        C c;
        for (int i = 0; i < N; ++i)
            c.push_front(i);
        c.reverse();
        assert(std::distance(c.begin(), c.end()) == N);
        typename C::const_iterator j = c.begin();
        for (int i = 0; i < N; ++i, ++j)
            assert(*j == i);
    }
}

TEST(reverse)
{
    for (int i = 0; i < 10; ++i)
        test_reverse<std::forward_list<int> >(i);
#if TEST_STD_VER >= 11
    for (int i = 0; i < 10; ++i)
        test_reverse<std::forward_list<int, min_allocator<int>> >(i);
#endif
}

namespace
{
    std::mt19937 randomness;

    template <class C>
    void test_sort(int N)
    {
        typedef typename C::value_type T;
        typedef std::vector<T> V;
        V v;
        for (int i = 0; i < N; ++i)
            v.push_back(i);
        std::shuffle(v.begin(), v.end(), randomness);
        C c(v.begin(), v.end());
        c.sort();
        assert(std::distance(c.begin(), c.end()) == N);
        typename C::const_iterator j = c.begin();
        for (int i = 0; i < N; ++i, ++j)
            assert(*j == i);
    }
}

TEST(sort)
{
    for (int i = 0; i < 40; ++i)
        test_sort<std::forward_list<int> >(i);
#if TEST_STD_VER >= 11
    for (int i = 0; i < 40; ++i)
        test_sort<std::forward_list<int, min_allocator<int>> >(i);
#endif
}

namespace
{
    template <class L>
    void do_unique(L& l, typename L::size_type expected)
    {
        typename L::size_type old_size = std::distance(l.begin(), l.end());
    #if TEST_STD_VER > 17
        ASSERT_SAME_TYPE(decltype(l.unique()), typename L::size_type);
        assert(l.unique() == expected);
    #else
        ASSERT_SAME_TYPE(decltype(l.unique()), void);
        l.unique();
    #endif
        assert(old_size - std::distance(l.begin(), l.end()) == expected);
    }
}

TEST(unique)
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {0, 5, 0, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 3);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 0, 0, 0};
        const T t2[] = {0};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 3);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 2);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c1;
        C c2;
        do_unique(c1, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 2);
        assert(c1 == c2);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {0, 5, 0, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 3);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 0, 0, 0};
        const T t2[] = {0};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 3);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 2);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c1;
        C c2;
        do_unique(c1, 0);
        assert(c1 == c2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        do_unique(c1, 2);
        assert(c1 == c2);
    }
#endif
}

TEST()
{

}

TEST()
{

}

#endif
