#ifdef TEST_HAS_CONTAINERS_MAP

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

TEST(at)
{
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        std::map<int, double> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        m.at(1) = -1.5;
        assert(m.at(1) == -1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
    #endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        const std::map<int, double> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
    #endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        m.at(1) = -1.5;
        assert(m.at(1) == -1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
    #endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        const std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
    #endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
#endif
}

TEST(empty)
{
    {
        typedef std::map<int, double> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        M m;
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#endif
}

TEST(iterator)
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
            V(4, 1),
            V(4, 1.5),
            V(4, 2),
            V(5, 1),
            V(5, 1.5),
            V(5, 2),
            V(6, 1),
            V(6, 1.5),
            V(6, 2),
            V(7, 1),
            V(7, 1.5),
            V(7, 2),
            V(8, 1),
            V(8, 1.5),
            V(8, 2)
        };
        std::map<int, double> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.rbegin(), m.rend())) == m.size());
        std::map<int, double>::iterator i;
        i = m.begin();
        std::map<int, double>::const_iterator k = i;
        assert(i == k);
        for (int j = 1; static_cast<std::size_t>(j) <= m.size(); ++j, ++i) {
            assert(i->first == j);
            assert(i->second == 1);
            i->second = 2.5;
            assert(i->second == 2.5);
        }
        assert(i == m.end());
        for (int j = (int)m.size(); j >= 1; --j) {
            --i;
            assert(i->first == j);
            assert(i->second == 2.5);
            i->second = 1;
            assert(i->second == 1);
        }
        assert(i == m.begin());
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
            V(4, 1),
            V(4, 1.5),
            V(4, 2),
            V(5, 1),
            V(5, 1.5),
            V(5, 2),
            V(6, 1),
            V(6, 1.5),
            V(6, 2),
            V(7, 1),
            V(7, 1.5),
            V(7, 2),
            V(8, 1),
            V(8, 1.5),
            V(8, 2)
        };
        const std::map<int, double> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.cbegin(), m.cend())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.rbegin(), m.rend())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.crbegin(), m.crend())) == m.size());
        std::map<int, double>::const_iterator i;
        i = m.begin();
        for (int j = 1; static_cast<std::size_t>(j) <= m.size(); ++j, ++i) {
            assert(i->first == j);
            assert(i->second == 1);
        }
        assert(i == m.end());
        for (int j = (int)m.size(); j >= 1; --j) {
            --i;
            assert(i->first == j);
            assert(i->second == 1);
        }
        assert(i == m.begin());
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
            V(4, 1),
            V(4, 1.5),
            V(4, 2),
            V(5, 1),
            V(5, 1.5),
            V(5, 2),
            V(6, 1),
            V(6, 1.5),
            V(6, 2),
            V(7, 1),
            V(7, 1.5),
            V(7, 2),
            V(8, 1),
            V(8, 1.5),
            V(8, 2)
        };
        std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.rbegin(), m.rend())) == m.size());
        std::map<int, double, std::less<int>, min_allocator<V>>::iterator i;
        i = m.begin();
        std::map<int, double, std::less<int>, min_allocator<V>>::const_iterator k = i;
        assert(i == k);
        for (int j = 1; static_cast<std::size_t>(j) <= m.size(); ++j, ++i) {
            assert(i->first == j);
            assert(i->second == 1);
            i->second = 2.5;
            assert(i->second == 2.5);
        }
        assert(i == m.end());
        for (int j = m.size(); j >= 1; --j) {
            --i;
            assert(i->first == j);
            assert(i->second == 2.5);
            i->second = 1;
            assert(i->second == 1);
        }
        assert(i == m.begin());
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
            V(4, 1),
            V(4, 1.5),
            V(4, 2),
            V(5, 1),
            V(5, 1.5),
            V(5, 2),
            V(6, 1),
            V(6, 1.5),
            V(6, 2),
            V(7, 1),
            V(7, 1.5),
            V(7, 2),
            V(8, 1),
            V(8, 1.5),
            V(8, 2)
        };
        const std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar + sizeof(ar) / sizeof(ar[0]));
        assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.cbegin(), m.cend())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.rbegin(), m.rend())) == m.size());
        assert(static_cast<std::size_t>(std::distance(m.crbegin(), m.crend())) == m.size());
        std::map<int, double, std::less<int>, min_allocator<V>>::const_iterator i;
        i = m.begin();
        for (int j = 1; static_cast<std::size_t>(j) <= m.size(); ++j, ++i) {
            assert(i->first == j);
            assert(i->second == 1);
        }
        assert(i == m.end());
        for (int j = m.size(); j >= 1; --j) {
            --i;
            assert(i->first == j);
            assert(i->second == 1);
        }
        assert(i == m.begin());
    }
#endif
#if TEST_STD_VER > 11
    { // N3644 testing
        typedef std::map<int, double> C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert(ii1 == ii2);
        assert(ii1 == ii4);

        assert(!(ii1 != ii2));

        assert((ii1 == cii));
        assert((cii == ii1));
        assert(!(ii1 != cii));
        assert(!(cii != ii1));
    }
#endif
}

TEST(size)
{
    {
        typedef std::map<int, double> M;
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
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
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

TEST(compare)
{
    {
        typedef test_less<int> C;
        const std::map<int, double, C> m(C(3));
        assert(m.empty());
        assert(m.begin() == m.end());
        assert(m.key_comp() == C(3));
    }
#if TEST_STD_VER >= 11
    {
        typedef test_less<int> C;
        const std::map<int, double, C, min_allocator<std::pair<const int, double>>> m(C(3));
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
        std::map<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::map<int, double, C, A> m = mo;
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == V(1, 1));
        assert(*std::next(mo.begin()) == V(2, 1));
        assert(*std::next(mo.begin(), 2) == V(3, 1));
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
        std::map<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::map<int, double, C, A> m = mo;
        assert(m.get_allocator() == A(-2));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == V(1, 1));
        assert(*std::next(mo.begin()) == V(2, 1));
        assert(*std::next(mo.begin(), 2) == V(3, 1));
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
        std::map<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A());
        std::map<int, double, C, A> m = mo;
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == V(1, 1));
        assert(*std::next(mo.begin()) == V(2, 1));
        assert(*std::next(mo.begin(), 2) == V(3, 1));
    }
#endif
}

TEST(default)
{
    {
        std::map<int, double> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
    {
        std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> m;
        assert(m.empty());
        assert(m.begin() == m.end());
    }
    {
        typedef explicit_allocator<std::pair<const int, double>> A;
        {
            std::map<int, double, std::less<int>, A> m;
            assert(m.empty());
            assert(m.begin() == m.end());
        }
        {
            A a;
            std::map<int, double, std::less<int>, A> m(a);
            assert(m.empty());
            assert(m.begin() == m.end());
        }
    }
    {
        std::map<int, double> m = {};
        assert(m.empty());
        assert(m.begin() == m.end());
    }
#endif
}

TEST(initializer_list)
{
    {
        typedef std::pair<const int, double> V;
        std::map<int, double> m =
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
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));
    }
    {
        typedef std::pair<const int, double> V;
        std::map<int, double, std::less<int>, min_allocator<V>> m =
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
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));
    }

}

TEST(move)
{
    typedef std::pair<const int, double> V;
    {
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::map<int, double, C, A> mo(C(5), A(7));
        std::map<int, double, C, A> m = std::move(mo);
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
        std::map<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A(7));
        std::map<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::map<int, double, C, A> mo(C(5), A());
        std::map<int, double, C, A> m = std::move(mo);
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
        std::map<int, double, C, A> mo(ar, ar + sizeof(ar) / sizeof(ar[0]), C(5), A());
        std::map<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(2, 1));
        assert(*std::next(m.begin(), 2) == V(3, 1));

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
}

TEST(clear)
{
    {
        typedef std::map<int, double> M;
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
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
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
        typedef std::map<int, DefaultOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(!r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::map<int, Emplaceable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef std::map<int, double> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
    {
        typedef std::map<int, DefaultOnly, std::less<int>, min_allocator<std::pair<const int, DefaultOnly>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(!r.second);
        assert(r.first == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::map<int, Emplaceable, std::less<int>, min_allocator<std::pair<const int, Emplaceable>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
}

TEST(emplace_hint)
{
    {
        typedef std::map<int, DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.end());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::map<int, Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef std::map<int, double> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.end(), M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
    {
        typedef std::map<int, DefaultOnly, std::less<int>, min_allocator<std::pair<const int, DefaultOnly>>> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.end());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple());
        assert(r == std::next(m.begin()));
        assert(m.size() == 2);
        assert(std::next(m.begin())->first == 1);
        assert(std::next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::map<int, Emplaceable, std::less<int>, min_allocator<std::pair<const int, Emplaceable>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(2),
            std::forward_as_tuple());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.end(), std::piecewise_construct,
            std::forward_as_tuple(1),
            std::forward_as_tuple(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.end(), M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
}

TEST(key_comp)
{
    typedef std::map<int, std::string> map_type;

    map_type m;
    std::pair<map_type::iterator, bool> p1 = m.insert(map_type::value_type(1, "abc"));
    std::pair<map_type::iterator, bool> p2 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.key_comp()(p1.first->first, p2.first->first));
    assert(!cm.key_comp()(p2.first->first, p1.first->first));

}

TEST(value_comp)
{
    typedef std::map<int, std::string> map_type;

    map_type m;
    std::pair<map_type::iterator, bool> p1 = m.insert(map_type::value_type(1, "abc"));
    std::pair<map_type::iterator, bool> p2 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.value_comp()(*p1.first, *p2.first));
    assert(!cm.value_comp()(*p2.first, *p1.first));

}

TEST(equal_range)
{
    {
        typedef std::pair<const int, double> V;
        typedef std::map<int, double> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
        {
            typedef std::pair<M::iterator, M::iterator> R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
#endif
#if TEST_STD_VER > 11
    {
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<>> M;
        typedef std::pair<M::iterator, M::iterator> R;

        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
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

        r = m.equal_range(C2Int(5));
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(C2Int(7));
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(C2Int(9));
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(C2Int(11));
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(C2Int(13));
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(C2Int(15));
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(C2Int(17));
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(C2Int(19));
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 8));
        r = m.equal_range(C2Int(4));
        assert(r.first == std::next(m.begin(), 0));
        assert(r.second == std::next(m.begin(), 0));
        r = m.equal_range(C2Int(6));
        assert(r.first == std::next(m.begin(), 1));
        assert(r.second == std::next(m.begin(), 1));
        r = m.equal_range(C2Int(8));
        assert(r.first == std::next(m.begin(), 2));
        assert(r.second == std::next(m.begin(), 2));
        r = m.equal_range(C2Int(10));
        assert(r.first == std::next(m.begin(), 3));
        assert(r.second == std::next(m.begin(), 3));
        r = m.equal_range(C2Int(12));
        assert(r.first == std::next(m.begin(), 4));
        assert(r.second == std::next(m.begin(), 4));
        r = m.equal_range(C2Int(14));
        assert(r.first == std::next(m.begin(), 5));
        assert(r.second == std::next(m.begin(), 5));
        r = m.equal_range(C2Int(16));
        assert(r.first == std::next(m.begin(), 6));
        assert(r.second == std::next(m.begin(), 6));
        r = m.equal_range(C2Int(18));
        assert(r.first == std::next(m.begin(), 7));
        assert(r.second == std::next(m.begin(), 7));
        r = m.equal_range(C2Int(20));
        assert(r.first == std::next(m.begin(), 8));
        assert(r.second == std::next(m.begin(), 8));
    }
#endif
}

TEST(find)
{
    {
        typedef std::pair<const int, double> V;
        typedef std::map<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(6, 6),
                V(7, 7),
                V(8, 8),
                V(9, 9),
                V(10, 10),
                V(11, 11),
                V(12, 12)
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
                V(5, 5),
                V(6, 6),
                V(7, 7),
                V(8, 8),
                V(9, 9),
                V(10, 10),
                V(11, 11),
                V(12, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(6, 6),
                V(7, 7),
                V(8, 8),
                V(9, 9),
                V(10, 10),
                V(11, 11),
                V(12, 12)
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
                V(5, 5),
                V(6, 6),
                V(7, 7),
                V(8, 8),
                V(9, 9),
                V(10, 10),
                V(11, 11),
                V(12, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<>> M;
        typedef M::iterator R;

        V ar[] =
        {
            V(5, 5),
            V(6, 6),
            V(7, 7),
            V(8, 8),
            V(9, 9),
            V(10, 10),
            V(11, 11),
            V(12, 12)
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

        r = m.find(C2Int(5));
        assert(r == m.begin());
        r = m.find(C2Int(6));
        assert(r == std::next(m.begin()));
        r = m.find(C2Int(7));
        assert(r == std::next(m.begin(), 2));
        r = m.find(C2Int(8));
        assert(r == std::next(m.begin(), 3));
        r = m.find(C2Int(9));
        assert(r == std::next(m.begin(), 4));
        r = m.find(C2Int(10));
        assert(r == std::next(m.begin(), 5));
        r = m.find(C2Int(11));
        assert(r == std::next(m.begin(), 6));
        r = m.find(C2Int(12));
        assert(r == std::next(m.begin(), 7));
        r = m.find(C2Int(4));
        assert(r == std::next(m.begin(), 8));
    }

    {
        typedef PrivateConstructor PC;
        typedef std::map<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m[PC::make(5)] = 5;
        m[PC::make(6)] = 6;
        m[PC::make(7)] = 7;
        m[PC::make(8)] = 8;
        m[PC::make(9)] = 9;
        m[PC::make(10)] = 10;
        m[PC::make(11)] = 11;
        m[PC::make(12)] = 12;

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

TEST(lower_bound)
{
    {
        typedef std::pair<const int, double> V;
        typedef std::map<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less <>> M;
        typedef M::iterator R;

        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
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

        r = m.lower_bound(C2Int(5));
        assert(r == m.begin());
        r = m.lower_bound(C2Int(7));
        assert(r == std::next(m.begin()));
        r = m.lower_bound(C2Int(9));
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(C2Int(11));
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(C2Int(13));
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(C2Int(15));
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(C2Int(17));
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(C2Int(19));
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(C2Int(4));
        assert(r == std::next(m.begin(), 0));
        r = m.lower_bound(C2Int(6));
        assert(r == std::next(m.begin(), 1));
        r = m.lower_bound(C2Int(8));
        assert(r == std::next(m.begin(), 2));
        r = m.lower_bound(C2Int(10));
        assert(r == std::next(m.begin(), 3));
        r = m.lower_bound(C2Int(12));
        assert(r == std::next(m.begin(), 4));
        r = m.lower_bound(C2Int(14));
        assert(r == std::next(m.begin(), 5));
        r = m.lower_bound(C2Int(16));
        assert(r == std::next(m.begin(), 6));
        r = m.lower_bound(C2Int(18));
        assert(r == std::next(m.begin(), 7));
        r = m.lower_bound(C2Int(20));
        assert(r == std::next(m.begin(), 8));
    }

    {
        typedef PrivateConstructor PC;
        typedef std::map<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m[PC::make(5)] = 5;
        m[PC::make(7)] = 6;
        m[PC::make(9)] = 7;
        m[PC::make(11)] = 8;
        m[PC::make(13)] = 9;
        m[PC::make(15)] = 10;
        m[PC::make(17)] = 11;
        m[PC::make(19)] = 12;

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

TEST(upper_bound)
{
    {
        typedef std::pair<const int, double> V;
        typedef std::map<int, double> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
                V(5, 5),
                V(7, 6),
                V(9, 7),
                V(11, 8),
                V(13, 9),
                V(15, 10),
                V(17, 11),
                V(19, 12)
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
        typedef std::pair<const int, double> V;
        typedef std::map<int, double, std::less<>> M;
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
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
        typedef PrivateConstructor PC;
        typedef std::map<PC, double, std::less<>> M;
        typedef M::iterator R;

        M m;
        m[PC::make(5)] = 5;
        m[PC::make(7)] = 6;
        m[PC::make(9)] = 7;
        m[PC::make(11)] = 8;
        m[PC::make(13)] = 9;
        m[PC::make(15)] = 10;
        m[PC::make(17)] = 11;
        m[PC::make(19)] = 12;

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
