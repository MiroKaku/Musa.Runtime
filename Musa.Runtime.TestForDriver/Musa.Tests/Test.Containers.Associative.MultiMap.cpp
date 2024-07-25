#ifdef TEST_HAS_CONTAINERS_MULTI_MAP

#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include "LLVM.TestSuite/min_allocator.h"
#include "LLVM.TestSuite/test_comparisons.h"
#include "LLVM.TestSuite/Counter.h"
#include "LLVM.TestSuite/MoveOnly.h"
#include "LLVM.TestSuite/DefaultOnly.h"
#include "LLVM.TestSuite/test_transparent_unordered.h"
#include "LLVM.TestSuite/private_constructor.h"


namespace
{
    template <class T>
    struct test_equal_to
    {
        int data_;
        explicit test_equal_to() : data_(0)
        {}
        explicit test_equal_to(int data) : data_(data)
        {}
        bool operator()(const T& a, const T& b) const
        {
            return a == b;
        }
        friend bool operator==(const test_equal_to& a, const test_equal_to& b)
        {
            return a.data_ == b.data_;
        }
    };

    template <class T>
    struct test_less
    {
        int data_;
        explicit test_less() : data_(0)
        {}
        explicit test_less(int data) : data_(data)
        {}
        bool operator()(const T& a, const T& b) const
        {
            return a < b;
        }
        friend bool operator==(const test_less& a, const test_less& b)
        {
            return a.data_ == b.data_;
        }
    };

    template <class T>
    class test_hash
    {
        int data_;
    public:
        explicit test_hash(int data = 0) : data_(data)
        {}

        std::size_t operator()(const T& x) const
        {
            return std::hash<T>()(x);
        }

        bool operator==(const test_hash& c) const
        {
            return data_ == c.data_;
        }
    };

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


TEST(compare)
{
    {
        typedef test_less<int> C;
        const std::multimap<int, double, C> m(C(3));
        assert(m.empty());
        assert(m.begin() == m.end());
        assert(m.key_comp() == C(3));
    }
#if TEST_STD_VER >= 11
    {
        typedef test_less<int> C;
        const std::multimap<int, double, C, min_allocator<std::pair<const int, double>>> m(C(3));
        assert(m.empty());
        assert(m.begin() == m.end());
        assert(m.key_comp() == C(3));
    }
#endif

}

TEST(copy)
{
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::multimap<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::multimap<int, double, C, A> m = mo;
        assert(m == mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_less<int> C;
        typedef other_allocator<V> A;
        std::multimap<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::multimap<int, double, C, A> m = mo;
        assert(m == mo);
        assert(m.get_allocator() == A(-2));
        assert(m.key_comp() == C(5));

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
    }
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::multimap<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A());
        std::multimap<int, double, C, A> m = mo;
        assert(m == mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
    }
#endif
}

TEST(default)
{
    {
        std::multimap<int, double> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
    {
        std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
    {
        typedef explicit_allocator<std::pair<const int, double>> A;
        {
            std::multimap<int, double, std::less<int>, A> m;
            assert(m.empty());
            assert(m.begin() == m.end());
        }
        {
            A a;
            std::multimap<int, double, std::less<int>, A> m(a);
            assert(m.empty());
            assert(m.begin() == m.end());
        }
    }
    {
        std::multimap<int, double> m = {};
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#endif
}

TEST(initializer_list)
{
    {
        typedef std::multimap<int, double> C;
        typedef C::value_type V;
        C m =
        {
            {1, 1},
            {1, 1.5},
            {1, 2},
            {2, 1},
            {2, 1.5},
            {2, 2},
            {3, 1},
            {3, 1.5},
            {3, 2}
        };
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        C::const_iterator i = m.cbegin();
        assert(*i == V(1, 1));
        assert(*++i == V(1, 1.5));
        assert(*++i == V(1, 2));
        assert(*++i == V(2, 1));
        assert(*++i == V(2, 1.5));
        assert(*++i == V(2, 2));
        assert(*++i == V(3, 1));
        assert(*++i == V(3, 1.5));
        assert(*++i == V(3, 2));
    }
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> C;
        typedef C::value_type V;
        C m =
        {
            {1, 1},
            {1, 1.5},
            {1, 2},
            {2, 1},
            {2, 1.5},
            {2, 2},
            {3, 1},
            {3, 1.5},
            {3, 2}
        };
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        C::const_iterator i = m.cbegin();
        assert(*i == V(1, 1));
        assert(*++i == V(1, 1.5));
        assert(*++i == V(1, 2));
        assert(*++i == V(2, 1));
        assert(*++i == V(2, 1.5));
        assert(*++i == V(2, 2));
        assert(*++i == V(3, 1));
        assert(*++i == V(3, 1.5));
        assert(*++i == V(3, 2));
    }
}

TEST(move)
{
    typedef std::pair<const int, double> V;
    {
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::multimap<int, double, C, A> mo(C(5), A(7));
        std::multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::multimap<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(1, 1.5));
        assert(*std::next(m.begin(), 2) == V(1, 2));
        assert(*std::next(m.begin(), 3) == V(2, 1));
        assert(*std::next(m.begin(), 4) == V(2, 1.5));
        assert(*std::next(m.begin(), 5) == V(2, 2));
        assert(*std::next(m.begin(), 6) == V(3, 1));
        assert(*std::next(m.begin(), 7) == V(3, 1.5));
        assert(*std::next(m.begin(), 8) == V(3, 2));

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::multimap<int, double, C, A> mo(C(5), A());
        std::multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::multimap<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A());
        std::multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(1, 1.5));
        assert(*std::next(m.begin(), 2) == V(1, 2));
        assert(*std::next(m.begin(), 3) == V(2, 1));
        assert(*std::next(m.begin(), 4) == V(2, 1.5));
        assert(*std::next(m.begin(), 5) == V(2, 2));
        assert(*std::next(m.begin(), 6) == V(3, 1));
        assert(*std::next(m.begin(), 7) == V(3, 1.5));
        assert(*std::next(m.begin(), 8) == V(3, 2));

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
}

TEST(clear)
{
    {
        typedef std::multimap<int, double> M;
        typedef std::pair<int, double> P;
        P ar[] =
        {
            P(1, 1.5),
            P(2, 2.5),
            P(3, 3.5),
            P(4, 4.5),
            P(5, 5.5),
            P(6, 6.5),
            P(7, 7.5),
            P(8, 8.5),
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef std::pair<int, double> P;
        P ar[] =
        {
            P(1, 1.5),
            P(2, 2.5),
            P(3, 3.5),
            P(4, 4.5),
            P(5, 5.5),
            P(6, 6.5),
            P(7, 7.5),
            P(8, 8.5),
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#endif
}

TEST(emplace)
{
    {
        typedef std::multimap<int, DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin(), 2));
        assert(m.size() == 3);
        assert(std::next(m.begin(), 2)->first == 1);
        assert(std::next(m.begin(), 2)->second == DefaultOnly());
        assert(DefaultOnly::count == 3);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::multimap<int, Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(3, 3.5));
        assert(r == std::next(m.begin()));
        assert(m.size() == 3);
        assert(r->first == 1);
        assert(r->second == Emplaceable(3, 3.5));
    }
    {
        typedef std::multimap<int, double> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
    {
        typedef std::multimap<int, DefaultOnly, std::less<int>, min_allocator<std::pair<const int, DefaultOnly>>> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin(), 2));
        assert(m.size() == 3);
        assert(std::next(m.begin(), 2)->first == 1);
        assert(std::next(m.begin(), 2)->second == DefaultOnly());
        assert(DefaultOnly::count == 3);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::multimap<int, Emplaceable, std::less<int>, min_allocator<std::pair<const int, Emplaceable>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(3, 3.5));
        assert(r == std::next(m.begin()));
        assert(m.size() == 3);
        assert(r->first == 1);
        assert(r->second == Emplaceable(3, 3.5));
    }
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
}

TEST(emplace_hint)
{
    {
        typedef std::multimap<int, DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin(), 2));
        assert(m.size() == 3);
        assert(std::next(m.begin(), 2)->first == 1);
        assert(std::next(m.begin(), 2)->second == DefaultOnly());
        assert(DefaultOnly::count == 3);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::multimap<int, Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace_hint(m.cbegin(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.cbegin(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(3, 3.5));
        assert(r == m.begin());
        assert(m.size() == 3);
        assert(r->first == 1);
        assert(r->second == Emplaceable(3, 3.5));
    }
    {
        typedef std::multimap<int, double> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
    {
        typedef std::multimap<int, DefaultOnly, std::less<int>, min_allocator<std::pair<const int, DefaultOnly>>> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin(), 2));
        assert(m.size() == 3);
        assert(std::next(m.begin(), 2)->first == 1);
        assert(std::next(m.begin(), 2)->second == DefaultOnly());
        assert(DefaultOnly::count == 3);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::multimap<int, Emplaceable, std::less<int>, min_allocator<std::pair<const int, Emplaceable>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), std::piecewise_construct,
            std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace_hint(m.cbegin(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.cbegin(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(3, 3.5));
        assert(r == m.begin());
        assert(m.size() == 3);
        assert(r->first == 1);
        assert(r->second == Emplaceable(3, 3.5));
    }
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
}

TEST(key_comp)
{
    typedef std::multimap<int, std::string> map_type;

    map_type m;
    map_type::iterator i1 = m.insert(map_type::value_type(1, "abc"));
    map_type::iterator i2 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.key_comp()(i1->first, i2->first));
    assert(!cm.key_comp()(i2->first, i1->first));
}

TEST(value_comp)
{
    typedef std::multimap<int, std::string> map_type;

    map_type m;
    map_type::iterator i1 = m.insert(map_type::value_type(1, "abc"));
    map_type::iterator i2 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.value_comp()(*i1, *i2));
    assert(!cm.value_comp()(*i2, *i1));

}

TEST(count)
{
    typedef std::pair<const int, double> V;
    {
        typedef std::multimap<int, double> M;
        {
            typedef M::size_type R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.count(4);
            assert(r == 0);
            r = m.count(5);
            assert(r == 3);
            r = m.count(6);
            assert(r == 0);
            r = m.count(7);
            assert(r == 3);
            r = m.count(8);
            assert(r == 0);
            r = m.count(9);
            assert(r == 3);
            r = m.count(10);
            assert(r == 0);
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        {
            typedef M::size_type R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.count(4);
            assert(r == 0);
            r = m.count(5);
            assert(r == 3);
            r = m.count(6);
            assert(r == 0);
            r = m.count(7);
            assert(r == 3);
            r = m.count(8);
            assert(r == 0);
            r = m.count(9);
            assert(r == 3);
            r = m.count(10);
            assert(r == 0);
        }
    }
#endif

#if TEST_STD_VER > 11
    {
        typedef std::multimap<int, double, std::less<>> M;
        typedef M::size_type R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.count(4);
        assert(r == 0);
        r = m.count(5);
        assert(r == 3);
        r = m.count(6);
        assert(r == 0);
        r = m.count(7);
        assert(r == 3);
        r = m.count(8);
        assert(r == 0);
        r = m.count(9);
        assert(r == 3);
        r = m.count(10);
        assert(r == 0);

        r = m.count(C2Int(4));
        assert(r == 0);
        r = m.count(C2Int(5));
        assert(r == 3);
        r = m.count(C2Int(6));
        assert(r == 0);
        r = m.count(C2Int(7));
        assert(r == 3);
        r = m.count(C2Int(8));
        assert(r == 0);
        r = m.count(C2Int(9));
        assert(r == 3);
        r = m.count(C2Int(10));
        assert(r == 0);
    }

    {
        typedef PrivateConstructor PC;
        typedef std::multimap<PC, double, std::less<>> M;
        typedef M::size_type R;

        M m;
        m.insert(std::make_pair<PC, double>(PC::make(5), 1));
        m.insert(std::make_pair<PC, double>(PC::make(5), 2));
        m.insert(std::make_pair<PC, double>(PC::make(5), 3));
        m.insert(std::make_pair<PC, double>(PC::make(7), 1));
        m.insert(std::make_pair<PC, double>(PC::make(7), 2));
        m.insert(std::make_pair<PC, double>(PC::make(7), 3));
        m.insert(std::make_pair<PC, double>(PC::make(9), 1));
        m.insert(std::make_pair<PC, double>(PC::make(9), 2));
        m.insert(std::make_pair<PC, double>(PC::make(9), 3));

        R r = m.count(4);
        assert(r == 0);
        r = m.count(5);
        assert(r == 3);
        r = m.count(6);
        assert(r == 0);
        r = m.count(7);
        assert(r == 3);
        r = m.count(8);
        assert(r == 0);
        r = m.count(9);
        assert(r == 3);
        r = m.count(10);
        assert(r == 0);
    }
#endif
}

TEST(find)
{

    typedef std::pair<const int, double> V;
    {
        typedef std::multimap<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == m.end());
            r = m.find(7);
            assert(r == std::next(m.begin(), 3));
            r = m.find(8);
            assert(r == m.end());
            r = m.find(9);
            assert(r == std::next(m.begin(), 6));
            r = m.find(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == m.end());
            r = m.find(7);
            assert(r == std::next(m.begin(), 3));
            r = m.find(8);
            assert(r == m.end());
            r = m.find(9);
            assert(r == std::next(m.begin(), 6));
            r = m.find(10);
            assert(r == m.end());
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == m.end());
            r = m.find(7);
            assert(r == std::next(m.begin(), 3));
            r = m.find(8);
            assert(r == m.end());
            r = m.find(9);
            assert(r == std::next(m.begin(), 6));
            r = m.find(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == m.end());
            r = m.find(7);
            assert(r == std::next(m.begin(), 3));
            r = m.find(8);
            assert(r == m.end());
            r = m.find(9);
            assert(r == std::next(m.begin(), 6));
            r = m.find(10);
            assert(r == m.end());
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef std::multimap<int, double, std::less<>> M;
        typedef M::iterator R;

        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == std::next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == std::next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());

        r = m.find(C2Int(5));
        assert(r == m.begin());
        r = m.find(C2Int(6));
        assert(r == m.end());
        r = m.find(C2Int(7));
        assert(r == std::next(m.begin(), 3));
        r = m.find(C2Int(8));
        assert(r == m.end());
        r = m.find(C2Int(9));
        assert(r == std::next(m.begin(), 6));
        r = m.find(C2Int(10));
        assert(r == m.end());
    }

    {
        typedef PrivateConstructor PC;
        typedef std::multimap<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(std::make_pair<PC, double>(PC::make(5), 1));
        m.insert(std::make_pair<PC, double>(PC::make(5), 2));
        m.insert(std::make_pair<PC, double>(PC::make(5), 3));
        m.insert(std::make_pair<PC, double>(PC::make(7), 1));
        m.insert(std::make_pair<PC, double>(PC::make(7), 2));
        m.insert(std::make_pair<PC, double>(PC::make(7), 3));
        m.insert(std::make_pair<PC, double>(PC::make(9), 1));
        m.insert(std::make_pair<PC, double>(PC::make(9), 2));
        m.insert(std::make_pair<PC, double>(PC::make(9), 3));

        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == std::next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == std::next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());
    }
#endif
}

TEST(equal_range)
{
    typedef std::pair<const int, double> V;
    {
        typedef std::multimap<int, double> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(4);
            assert(r.first == m.begin());
            assert(r.second == m.begin());
            r = m.equal_range(5);
            assert(r.first == m.begin());
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 9));
            r = m.equal_range(10);
            assert(r.first == m.end());
            assert(r.second == m.end());
        }
        {
            typedef std::pair<M::const_iterator, M::const_iterator> R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(4);
            assert(r.first == m.begin());
            assert(r.second == m.begin());
            r = m.equal_range(5);
            assert(r.first == m.begin());
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 9));
            r = m.equal_range(10);
            assert(r.first == m.end());
            assert(r.second == m.end());
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(4);
            assert(r.first == m.begin());
            assert(r.second == m.begin());
            r = m.equal_range(5);
            assert(r.first == m.begin());
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 9));
            r = m.equal_range(10);
            assert(r.first == m.end());
            assert(r.second == m.end());
        }
        {
            typedef std::pair<M::const_iterator, M::const_iterator> R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(4);
            assert(r.first == m.begin());
            assert(r.second == m.begin());
            r = m.equal_range(5);
            assert(r.first == m.begin());
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 9));
            r = m.equal_range(10);
            assert(r.first == m.end());
            assert(r.second == m.end());
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef std::multimap<int, double, std::less<>> M;

        typedef std::pair<M::iterator, M::iterator> R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());

        r = m.equal_range(C2Int(4));
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(C2Int(5));
        assert(r.first == m.begin());
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(C2Int(6));
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(C2Int(7));
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(C2Int(8));
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(C2Int(9));
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 9));
        r = m.equal_range(C2Int(10));
        assert(r.first == m.end());
        assert(r.second == m.end());
    }

    {
        typedef PrivateConstructor PC;
        typedef std::multimap<PC, double, std::less<>> M;
        typedef std::pair<M::iterator, M::iterator> R;

        M m;
        m.insert(std::make_pair<PC, double>(PC::make(5), 1));
        m.insert(std::make_pair<PC, double>(PC::make(5), 2));
        m.insert(std::make_pair<PC, double>(PC::make(5), 3));
        m.insert(std::make_pair<PC, double>(PC::make(7), 1));
        m.insert(std::make_pair<PC, double>(PC::make(7), 2));
        m.insert(std::make_pair<PC, double>(PC::make(7), 3));
        m.insert(std::make_pair<PC, double>(PC::make(9), 1));
        m.insert(std::make_pair<PC, double>(PC::make(9), 2));
        m.insert(std::make_pair<PC, double>(PC::make(9), 3));

        //  assert(m.size() == 9);
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());
    }
#endif
}

TEST(lower_bound)
{
    typedef std::pair<const int, double> V;
    {
        typedef std::multimap<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == m.begin());
            r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == m.begin());
            r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(10);
            assert(r == m.end());
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == m.begin());
            r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == m.begin());
            r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(10);
            assert(r == m.end());
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef std::multimap<int, double, std::less<>> M;
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.lower_bound(4);
        assert(r == m.begin());
        r = m.lower_bound(5);
        assert(r == m.begin());
        r = m.lower_bound(6);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(7);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(8);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(9);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(10);
        assert(r == m.end());

        r = m.lower_bound(C2Int(4));
        assert(r == m.begin());
        r = m.lower_bound(C2Int(5));
        assert(r == m.begin());
        r = m.lower_bound(C2Int(6));
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(C2Int(7));
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(C2Int(8));
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(C2Int(9));
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(C2Int(10));
        assert(r == m.end());
    }

    {
        typedef PrivateConstructor PC;
        typedef std::multimap<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(std::make_pair<PC, double>(PC::make(5), 1));
        m.insert(std::make_pair<PC, double>(PC::make(5), 2));
        m.insert(std::make_pair<PC, double>(PC::make(5), 3));
        m.insert(std::make_pair<PC, double>(PC::make(7), 1));
        m.insert(std::make_pair<PC, double>(PC::make(7), 2));
        m.insert(std::make_pair<PC, double>(PC::make(7), 3));
        m.insert(std::make_pair<PC, double>(PC::make(9), 1));
        m.insert(std::make_pair<PC, double>(PC::make(9), 2));
        m.insert(std::make_pair<PC, double>(PC::make(9), 3));

        R r = m.lower_bound(4);
        assert(r == m.begin());
        r = m.lower_bound(5);
        assert(r == m.begin());
        r = m.lower_bound(6);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(7);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(8);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(9);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(10);
        assert(r == m.end());
    }

#endif
}

TEST(upper_bound)
{
    typedef std::pair<const int, double> V;
    {
        typedef std::multimap<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(4);
            assert(r == m.begin());
            r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 9));
            r = m.upper_bound(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(4);
            assert(r == m.begin());
            r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 9));
            r = m.upper_bound(10);
            assert(r == m.end());
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(4);
            assert(r == m.begin());
            r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 9));
            r = m.upper_bound(10);
            assert(r == m.end());
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                V(5, 1),
                V(5, 2),
                V(5, 3),
                V(7, 1),
                V(7, 2),
                V(7, 3),
                V(9, 1),
                V(9, 2),
                V(9, 3)
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(4);
            assert(r == m.begin());
            r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 9));
            r = m.upper_bound(10);
            assert(r == m.end());
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef std::multimap<int, double, std::less<>> M;
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.upper_bound(4);
        assert(r == m.begin());
        r = m.upper_bound(5);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(6);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(7);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(8);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(9);
        assert(r == std::next(m.begin(), 9));
        r = m.upper_bound(10);
        assert(r == m.end());

        r = m.upper_bound(C2Int(4));
        assert(r == m.begin());
        r = m.upper_bound(C2Int(5));
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(C2Int(6));
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(C2Int(7));
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(C2Int(8));
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(C2Int(9));
        assert(r == std::next(m.begin(), 9));
        r = m.upper_bound(C2Int(10));
    }

    {
        typedef PrivateConstructor PC;
        typedef std::multimap<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(std::make_pair<PC, double>(PC::make(5), 1));
        m.insert(std::make_pair<PC, double>(PC::make(5), 2));
        m.insert(std::make_pair<PC, double>(PC::make(5), 3));
        m.insert(std::make_pair<PC, double>(PC::make(7), 1));
        m.insert(std::make_pair<PC, double>(PC::make(7), 2));
        m.insert(std::make_pair<PC, double>(PC::make(7), 3));
        m.insert(std::make_pair<PC, double>(PC::make(9), 1));
        m.insert(std::make_pair<PC, double>(PC::make(9), 2));
        m.insert(std::make_pair<PC, double>(PC::make(9), 3));

        R r = m.upper_bound(4);
        assert(r == m.begin());
        r = m.upper_bound(5);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(6);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(7);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(8);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(9);
        assert(r == std::next(m.begin(), 9));
        r = m.upper_bound(10);
        assert(r == m.end());
    }

#endif
}

TEST(empty)
{
    {
        typedef std::multimap<int, double> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#endif
}

TEST(size)
{
    {
        typedef std::multimap<int, double> M;
        M m;
        assert(m.size() == 0);
        m.insert(M::value_type(2, 1.5));
        assert(m.size() == 1);
        m.insert(M::value_type(1, 1.5));
        assert(m.size() == 2);
        m.insert(M::value_type(3, 1.5));
        assert(m.size() == 3);
        m.erase(m.begin());
        assert(m.size() == 2);
        m.erase(m.begin());
        assert(m.size() == 1);
        m.erase(m.begin());
        assert(m.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        M m;
        assert(m.size() == 0);
        m.insert(M::value_type(2, 1.5));
        assert(m.size() == 1);
        m.insert(M::value_type(1, 1.5));
        assert(m.size() == 2);
        m.insert(M::value_type(3, 1.5));
        assert(m.size() == 3);
        m.erase(m.begin());
        assert(m.size() == 2);
        m.erase(m.begin());
        assert(m.size() == 1);
        m.erase(m.begin());
        assert(m.size() == 0);
    }
#endif
}

#endif
