#ifdef TEST_HAS_CONTAINERS_SET

#include <algorithm>
#include <set>
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
    typedef test_less<int> C;
    const std::set<int, C> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    assert(m.value_comp() == C(3));
}

TEST(copy)
{
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::set<int, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::set<int, C, A> m = mo;
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == 1);
        assert(*std::next(mo.begin()) == 2);
        assert(*std::next(mo.begin(), 2) == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef other_allocator<V> A;
        std::set<int, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::set<int, C, A> m = mo;
        assert(m.get_allocator() == A(-2));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == 1);
        assert(*std::next(mo.begin()) == 2);
        assert(*std::next(mo.begin(), 2) == 3);
    }
#endif
}

TEST(default)
{
    {
        std::set<int> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
    {
        std::set<int, std::less<int>, min_allocator<int>> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
    {
        typedef explicit_allocator<int> A;
        {
            std::set<int, std::less<int>, A> m;
            assert(m.empty());
            assert(m.begin() == m.end());
        }
        {
            A a;
            std::set<int, std::less<int>, A> m(a);
            assert(m.empty());
            assert(m.begin() == m.end());
        }
    }
    {
        std::set<int> m = {};
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#endif
}

TEST(initializer_list)
{
    {
        typedef std::set<int> C;
        typedef C::value_type V;
        C m = {1, 2, 3, 4, 5, 6};
        assert(m.size() == 6);
        assert(std::distance(m.begin(), m.end()) == 6);
        C::const_iterator i = m.cbegin();
        assert(*i == V(1));
        assert(*++i == V(2));
        assert(*++i == V(3));
        assert(*++i == V(4));
        assert(*++i == V(5));
        assert(*++i == V(6));
    }
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> C;
        typedef C::value_type V;
        C m = {1, 2, 3, 4, 5, 6};
        assert(m.size() == 6);
        assert(std::distance(m.begin(), m.end()) == 6);
        C::const_iterator i = m.cbegin();
        assert(*i == V(1));
        assert(*++i == V(2));
        assert(*++i == V(3));
        assert(*++i == V(4));
        assert(*++i == V(5));
        assert(*++i == V(6));
    }
}

TEST(move)
{
    {
        typedef int V;
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::set<int, C, A> mo(C(5), A(7));
        std::set<int, C, A> m = std::move(mo);
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
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::set<int, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::set<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::set<int, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A());
        std::set<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
}

TEST(clear)
{
    {
        typedef std::set<int> M;
        typedef int V;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#endif
}

TEST(count)
{
    {
        typedef int V;
        typedef std::set<int> M;
        typedef M::size_type R;
        V ar[] =
        {
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
        };
        const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.count(5);
        assert(r == 1);
        r = m.count(6);
        assert(r == 1);
        r = m.count(7);
        assert(r == 1);
        r = m.count(8);
        assert(r == 1);
        r = m.count(9);
        assert(r == 1);
        r = m.count(10);
        assert(r == 1);
        r = m.count(11);
        assert(r == 1);
        r = m.count(12);
        assert(r == 1);
        r = m.count(4);
        assert(r == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef M::size_type R;
        V ar[] =
        {
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
        };
        const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.count(5);
        assert(r == 1);
        r = m.count(6);
        assert(r == 1);
        r = m.count(7);
        assert(r == 1);
        r = m.count(8);
        assert(r == 1);
        r = m.count(9);
        assert(r == 1);
        r = m.count(10);
        assert(r == 1);
        r = m.count(11);
        assert(r == 1);
        r = m.count(12);
        assert(r == 1);
        r = m.count(4);
        assert(r == 0);
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef int V;
        typedef std::set<int, std::less<>> M;
        typedef M::size_type R;
        V ar[] =
        {
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
        };
        const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.count(5);
        assert(r == 1);
        r = m.count(6);
        assert(r == 1);
        r = m.count(7);
        assert(r == 1);
        r = m.count(8);
        assert(r == 1);
        r = m.count(9);
        assert(r == 1);
        r = m.count(10);
        assert(r == 1);
        r = m.count(11);
        assert(r == 1);
        r = m.count(12);
        assert(r == 1);
        r = m.count(4);
        assert(r == 0);
    }
    {
        typedef PrivateConstructor V;
        typedef std::set<V, std::less<>> M;
        typedef M::size_type R;

        M m;
        m.insert(V::make(5));
        m.insert(V::make(6));
        m.insert(V::make(7));
        m.insert(V::make(8));
        m.insert(V::make(9));
        m.insert(V::make(10));
        m.insert(V::make(11));
        m.insert(V::make(12));

        const M& mc = m;

        R r = mc.count(5);
        assert(r == 1);
        r = mc.count(6);
        assert(r == 1);
        r = mc.count(7);
        assert(r == 1);
        r = mc.count(8);
        assert(r == 1);
        r = mc.count(9);
        assert(r == 1);
        r = mc.count(10);
        assert(r == 1);
        r = mc.count(11);
        assert(r == 1);
        r = mc.count(12);
        assert(r == 1);
        r = mc.count(4);
        assert(r == 0);
    }
#endif
}

TEST(emplace)
{
    {
        typedef std::set<DefaultOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace();
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::set<Emplaceable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace(2, 3.5);
        assert(r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(*r.first == Emplaceable(2, 3.5));
        r = m.emplace(2, 3.5);
        assert(!r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(*r.first == Emplaceable(2, 3.5));
    }
    {
        typedef std::set<int> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);
    }
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);
    }

}

TEST(emplace_hint)
{
    {
        typedef std::set<DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace_hint(m.cbegin());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::set<Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace_hint(m.cend(), 2, 3.5);
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.cbegin(), 2, 3.5);
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
    }
    {
        typedef std::set<int> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
}

TEST(empty)
{
    {
        typedef std::set<int> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#endif
}

TEST(equal_range)
{
    {
        typedef int V;
        typedef std::set<int> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(5);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(11);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(13);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(15);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(17);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(19);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 8));
            r = m.equal_range(4);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 0));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(10);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(12);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(14);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(16);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(18);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(20);
            assert(r.first == std::next(m.begin(), 8));
            assert(r.second == std::next(m.begin(), 8));
        }
        {
            typedef std::pair<M::const_iterator, M::const_iterator> R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(5);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(11);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(13);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(15);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(17);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(19);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 8));
            r = m.equal_range(4);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 0));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(10);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(12);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(14);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(16);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(18);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(20);
            assert(r.first == std::next(m.begin(), 8));
            assert(r.second == std::next(m.begin(), 8));
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef std::pair<M::iterator, M::iterator> R;
        V ar[] =
        {
            5,
            7,
            9,
            11,
            13,
            15,
            17,
            19
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.equal_range(5);
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(7);
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(9);
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(11);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(13);
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(15);
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(17);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(19);
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 8));
        r = m.equal_range(4);
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 0));
        r = m.equal_range(6);
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(8);
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(10);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(12);
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(14);
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(16);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(18);
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(20);
        assert(r.first == std::next(m.begin(), 8));
        assert(r.second == std::next(m.begin(), 8));
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef int V;
        typedef std::set<V, std::less<>> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.equal_range(5);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(7);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(9);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(11);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(13);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(15);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(17);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(19);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 8));
            r = m.equal_range(4);
            assert(r.first == std::next(m.begin(), 0));
            assert(r.second == std::next(m.begin(), 0));
            r = m.equal_range(6);
            assert(r.first == std::next(m.begin(), 1));
            assert(r.second == std::next(m.begin(), 1));
            r = m.equal_range(8);
            assert(r.first == std::next(m.begin(), 2));
            assert(r.second == std::next(m.begin(), 2));
            r = m.equal_range(10);
            assert(r.first == std::next(m.begin(), 3));
            assert(r.second == std::next(m.begin(), 3));
            r = m.equal_range(12);
            assert(r.first == std::next(m.begin(), 4));
            assert(r.second == std::next(m.begin(), 4));
            r = m.equal_range(14);
            assert(r.first == std::next(m.begin(), 5));
            assert(r.second == std::next(m.begin(), 5));
            r = m.equal_range(16);
            assert(r.first == std::next(m.begin(), 6));
            assert(r.second == std::next(m.begin(), 6));
            r = m.equal_range(18);
            assert(r.first == std::next(m.begin(), 7));
            assert(r.second == std::next(m.begin(), 7));
            r = m.equal_range(20);
            assert(r.first == std::next(m.begin(), 8));
            assert(r.second == std::next(m.begin(), 8));
        }
    }
    {
        typedef PrivateConstructor V;
        typedef std::set<V, std::less<>> M;
        typedef std::pair<M::iterator, M::iterator> R;

        M m;
        m.insert(V::make(5));
        m.insert(V::make(7));
        m.insert(V::make(9));
        m.insert(V::make(11));
        m.insert(V::make(13));
        m.insert(V::make(15));
        m.insert(V::make(17));
        m.insert(V::make(19));

        R r = m.equal_range(5);
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(7);
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(9);
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(11);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(13);
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(15);
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(17);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(19);
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 8));
        r = m.equal_range(4);
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 0));
        r = m.equal_range(6);
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(8);
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(10);
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(12);
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(14);
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(16);
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(18);
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(20);
        assert(r.first == std::next(m.begin(), 8));
        assert(r.second == std::next(m.begin(), 8));
    }
#endif
}

TEST(erase_key)
{
    {
        typedef std::set<int> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 4);
        assert(*std::next(m.begin(), 4) == 5);
        assert(*std::next(m.begin(), 5) == 6);
        assert(*std::next(m.begin(), 6) == 7);
        assert(*std::next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 4);
        assert(*std::next(m.begin(), 4) == 5);
        assert(*std::next(m.begin(), 5) == 6);
        assert(*std::next(m.begin(), 6) == 7);
        assert(*std::next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#endif
}

TEST(find)
{
    {
        typedef int V;
        typedef std::set<int> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == std::next(m.begin()));
            r = m.find(7);
            assert(r == std::next(m.begin(), 2));
            r = m.find(8);
            assert(r == std::next(m.begin(), 3));
            r = m.find(9);
            assert(r == std::next(m.begin(), 4));
            r = m.find(10);
            assert(r == std::next(m.begin(), 5));
            r = m.find(11);
            assert(r == std::next(m.begin(), 6));
            r = m.find(12);
            assert(r == std::next(m.begin(), 7));
            r = m.find(4);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == std::next(m.begin()));
            r = m.find(7);
            assert(r == std::next(m.begin(), 2));
            r = m.find(8);
            assert(r == std::next(m.begin(), 3));
            r = m.find(9);
            assert(r == std::next(m.begin(), 4));
            r = m.find(10);
            assert(r == std::next(m.begin(), 5));
            r = m.find(11);
            assert(r == std::next(m.begin(), 6));
            r = m.find(12);
            assert(r == std::next(m.begin(), 7));
            r = m.find(4);
            assert(r == std::next(m.begin(), 8));
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == std::next(m.begin()));
            r = m.find(7);
            assert(r == std::next(m.begin(), 2));
            r = m.find(8);
            assert(r == std::next(m.begin(), 3));
            r = m.find(9);
            assert(r == std::next(m.begin(), 4));
            r = m.find(10);
            assert(r == std::next(m.begin(), 5));
            r = m.find(11);
            assert(r == std::next(m.begin(), 6));
            r = m.find(12);
            assert(r == std::next(m.begin(), 7));
            r = m.find(4);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == std::next(m.begin()));
            r = m.find(7);
            assert(r == std::next(m.begin(), 2));
            r = m.find(8);
            assert(r == std::next(m.begin(), 3));
            r = m.find(9);
            assert(r == std::next(m.begin(), 4));
            r = m.find(10);
            assert(r == std::next(m.begin(), 5));
            r = m.find(11);
            assert(r == std::next(m.begin(), 6));
            r = m.find(12);
            assert(r == std::next(m.begin(), 7));
            r = m.find(4);
            assert(r == std::next(m.begin(), 8));
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef int V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        V ar[] =
        {
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == std::next(m.begin()));
        r = m.find(7);
        assert(r == std::next(m.begin(), 2));
        r = m.find(8);
        assert(r == std::next(m.begin(), 3));
        r = m.find(9);
        assert(r == std::next(m.begin(), 4));
        r = m.find(10);
        assert(r == std::next(m.begin(), 5));
        r = m.find(11);
        assert(r == std::next(m.begin(), 6));
        r = m.find(12);
        assert(r == std::next(m.begin(), 7));
        r = m.find(4);
        assert(r == std::next(m.begin(), 8));
    }

    {
        typedef PrivateConstructor V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(V::make(5));
        m.insert(V::make(6));
        m.insert(V::make(7));
        m.insert(V::make(8));
        m.insert(V::make(9));
        m.insert(V::make(10));
        m.insert(V::make(11));
        m.insert(V::make(12));

        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == std::next(m.begin()));
        r = m.find(7);
        assert(r == std::next(m.begin(), 2));
        r = m.find(8);
        assert(r == std::next(m.begin(), 3));
        r = m.find(9);
        assert(r == std::next(m.begin(), 4));
        r = m.find(10);
        assert(r == std::next(m.begin(), 5));
        r = m.find(11);
        assert(r == std::next(m.begin(), 6));
        r = m.find(12);
        assert(r == std::next(m.begin(), 7));
        r = m.find(4);
        assert(r == std::next(m.begin(), 8));
    }
#endif
}

TEST(insert_rv)
{
    {
        typedef std::set<MoveOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.insert(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);

        r = m.insert(M::value_type(1));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(*r.first == 1);

        r = m.insert(M::value_type(3));
        assert(r.second);
        assert(r.first == std::prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);

        r = m.insert(M::value_type(3));
        assert(!r.second);
        assert(r.first == std::prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);
    }
    {
        typedef std::set<MoveOnly, std::less<MoveOnly>, min_allocator<MoveOnly>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.insert(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);

        r = m.insert(M::value_type(1));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(*r.first == 1);

        r = m.insert(M::value_type(3));
        assert(r.second);
        assert(r.first == std::prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);

        r = m.insert(M::value_type(3));
        assert(!r.second);
        assert(r.first == std::prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);
    }
}

TEST(lower_bound)
{
    {
        typedef int V;
        typedef std::set<int> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(7);
            assert(r == std::next(m.begin()));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(11);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(13);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(15);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(17);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(19);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(7);
            assert(r == std::next(m.begin()));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(11);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(13);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(15);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(17);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(19);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(7);
            assert(r == std::next(m.begin()));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(11);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(13);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(15);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(17);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(19);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.lower_bound(5);
            assert(r == m.begin());
            r = m.lower_bound(7);
            assert(r == std::next(m.begin()));
            r = m.lower_bound(9);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(11);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(13);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(15);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(17);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(19);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.lower_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.lower_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.lower_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.lower_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.lower_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.lower_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.lower_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef int V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        V ar[] =
        {
            5,
            7,
            9,
            11,
            13,
            15,
            17,
            19
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.lower_bound(5);
        assert(r == m.begin());
        r = m.lower_bound(7);
        assert(r == std::next(m.begin()));
        r = m.lower_bound(9);
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(11);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(13);
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(15);
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(17);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(19);
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(4);
        assert(r == std::next(m.begin(), 0));
        r = m.lower_bound(6);
        assert(r == std::next(m.begin(), 1));
        r = m.lower_bound(8);
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(10);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(12);
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(14);
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(16);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(18);
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(20);
        assert(r == std::next(m.begin(), 8));
    }

    {
        typedef PrivateConstructor V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(V::make(5));
        m.insert(V::make(7));
        m.insert(V::make(9));
        m.insert(V::make(11));
        m.insert(V::make(13));
        m.insert(V::make(15));
        m.insert(V::make(17));
        m.insert(V::make(19));

        R r = m.lower_bound(5);
        assert(r == m.begin());
        r = m.lower_bound(7);
        assert(r == std::next(m.begin()));
        r = m.lower_bound(9);
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(11);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(13);
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(15);
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(17);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(19);
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(4);
        assert(r == std::next(m.begin(), 0));
        r = m.lower_bound(6);
        assert(r == std::next(m.begin(), 1));
        r = m.lower_bound(8);
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(10);
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(12);
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(14);
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(16);
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(18);
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(20);
        assert(r == std::next(m.begin(), 8));
    }
#endif

}

TEST(size)
{
    {
        typedef std::set<int> M;
        M m;
        assert(m.size() == 0);
        m.insert(M::value_type(2));
        assert(m.size() == 1);
        m.insert(M::value_type(1));
        assert(m.size() == 2);
        m.insert(M::value_type(3));
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
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        M m;
        assert(m.size() == 0);
        m.insert(M::value_type(2));
        assert(m.size() == 1);
        m.insert(M::value_type(1));
        assert(m.size() == 2);
        m.insert(M::value_type(3));
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

TEST(upper_bound)
{

    {
        typedef int V;
        typedef std::set<int> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(11);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(13);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(15);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(17);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(19);
            assert(r == std::next(m.begin(), 8));
            r = m.upper_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(11);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(13);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(15);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(17);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(19);
            assert(r == std::next(m.begin(), 8));
            r = m.upper_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(11);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(13);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(15);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(17);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(19);
            assert(r == std::next(m.begin(), 8));
            r = m.upper_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                7,
                9,
                11,
                13,
                15,
                17,
                19
            };
            const M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
            R r = m.upper_bound(5);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(7);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(9);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(11);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(13);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(15);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(17);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(19);
            assert(r == std::next(m.begin(), 8));
            r = m.upper_bound(4);
            assert(r == std::next(m.begin(), 0));
            r = m.upper_bound(6);
            assert(r == std::next(m.begin(), 1));
            r = m.upper_bound(8);
            assert(r == std::next(m.begin(), 2));
            r = m.upper_bound(10);
            assert(r == std::next(m.begin(), 3));
            r = m.upper_bound(12);
            assert(r == std::next(m.begin(), 4));
            r = m.upper_bound(14);
            assert(r == std::next(m.begin(), 5));
            r = m.upper_bound(16);
            assert(r == std::next(m.begin(), 6));
            r = m.upper_bound(18);
            assert(r == std::next(m.begin(), 7));
            r = m.upper_bound(20);
            assert(r == std::next(m.begin(), 8));
        }
    }
#endif
#if TEST_STD_VER > 11
    {
        typedef int V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        V ar[] =
        {
            5,
            7,
            9,
            11,
            13,
            15,
            17,
            19
        };
        M m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        R r = m.upper_bound(5);
        assert(r == std::next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == std::next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == std::next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == std::next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == std::next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == std::next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == std::next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == std::next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == std::next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == std::next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == std::next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == std::next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == std::next(m.begin(), 8));
    }

    {
        typedef PrivateConstructor V;
        typedef std::set<V, std::less<>> M;
        typedef M::iterator R;

        M m;
        m.insert(V::make(5));
        m.insert(V::make(7));
        m.insert(V::make(9));
        m.insert(V::make(11));
        m.insert(V::make(13));
        m.insert(V::make(15));
        m.insert(V::make(17));
        m.insert(V::make(19));

        R r = m.upper_bound(5);
        assert(r == std::next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == std::next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == std::next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == std::next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == std::next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == std::next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == std::next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == std::next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == std::next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == std::next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == std::next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == std::next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == std::next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == std::next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == std::next(m.begin(), 8));
    }
#endif
}

#endif
