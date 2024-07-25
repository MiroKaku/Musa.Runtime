#ifdef TEST_HAS_CONTAINERS_UNORDER_MULTI_MAP

#include <algorithm>
#include <unordered_map>
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

    // Check consecutive equal values in an unordered_multiset iterator
    template <typename Iter>
    void CheckConsecutiveValues(Iter pos, Iter end, typename Iter::value_type value, std::size_t count)
    {
        for (std::size_t i = 0; i < count; ++i) {
            assert(pos != end);
            assert(*pos == value);
            ++pos;
        }
        assert(pos == end || *pos != value);
    }

    // Check consecutive equal keys in an unordered_multimap iterator
    template <typename Iter>
    void CheckConsecutiveKeys(Iter pos, Iter end, typename Iter::value_type::first_type key, std::multiset<typename Iter::value_type::second_type>& values)
    {
        while (!values.empty()) {
            assert(pos != end);
            assert(pos->first == key);
            assert(values.find(pos->second) != values.end());
            values.erase(values.find(pos->second));
            ++pos;
        }
        assert(pos == end || pos->first != key);
    }
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

TEST(assign_copy)
{
    {
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A(4)
        );
        c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(4));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<const int, std::string> P;
        const P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        C* p = &c;
        c = *p;
        assert(c.size() == 6);
        assert(std::is_permutation(c.begin(), c.end(), a));
    }
    {
        typedef other_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A(4)
        );
        c = c0;
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A()
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A()
        );
        c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif
}

TEST(assign_init)
{
    {
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        C c = {
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        c = {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        C c = {
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        c = {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
}

TEST(assign_move)
{
    {
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A(4)
        );
        c = std::move(c0);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A(10)
        );
        C::iterator it0 = c0.begin();
        c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef other_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A(4)
        );
        C::iterator it0 = c0.begin();
        c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A()
        );
        C c(a, a + 2,
            7,
            test_hash<int>(2),
            test_equal_to<int>(3),
            A()
        );
        C::iterator it0 = c0.begin();
        c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }

}

TEST(copy)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<std::pair<const int, std::string> >(10)
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >(10)));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            other_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            other_allocator<std::pair<const int, std::string> >(10)
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (other_allocator<std::pair<const int, std::string> >(-2)));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<std::pair<const int, std::string> >()
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif
}

TEST(copy_alloc)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<std::pair<const int, std::string> >(10)
        );
        C c(c0, test_allocator<std::pair<const int, std::string> >(5));
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >(5)));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<std::pair<const int, std::string> >()
        );
        C c(c0, min_allocator<std::pair<const int, std::string> >());
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef explicit_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A{}
        );
        C c(c0, A{});
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        s.insert("three");
        CheckConsecutiveKeys<C::const_iterator>(c.find(3), c.end(), 3, s);
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(4), c.end(), 4, s);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A{});
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif
}

TEST(default)
{
    {
        typedef std::unordered_multimap<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const NotConstructible, NotConstructible> >()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const NotConstructible, NotConstructible> >()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef explicit_allocator<std::pair<const NotConstructible, NotConstructible>> A;
        typedef std::unordered_multimap<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            A
        > C;
        {
            C c;
            LIBCPP_ASSERT(c.bucket_count() == 0);
            assert(c.hash_function() == test_hash<NotConstructible>());
            assert(c.key_eq() == test_equal_to<NotConstructible>());
            assert(c.get_allocator() == A());
            assert(c.size() == 0);
            assert(c.empty());
            assert(std::distance(c.begin(), c.end()) == 0);
            assert(c.load_factor() == 0);
            assert(c.max_load_factor() == 1);
        }
        {
            A a;
            C c(a);
            LIBCPP_ASSERT(c.bucket_count() == 0);
            assert(c.hash_function() == test_hash<NotConstructible>());
            assert(c.key_eq() == test_equal_to<NotConstructible>());
            assert(c.get_allocator() == a);
            assert(c.size() == 0);
            assert(c.empty());
            assert(std::distance(c.begin(), c.end()) == 0);
            assert(c.load_factor() == 0);
            assert(c.max_load_factor() == 1);
        }
    }
    {
        std::unordered_multimap<int, int> c = {};
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
}

TEST(init)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c = {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >()));
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c = {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));
    }
#if TEST_STD_VER > 11
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef test_hash<int> HF;
        typedef test_equal_to<int> Comp;
        typedef std::unordered_multimap<int, std::string, HF, Comp, A> C;

        A a(42);
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            }, 12, a);
        assert(c.bucket_count() >= 12);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == HF());
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
    }
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef test_hash<int> HF;
        typedef test_equal_to<int> Comp;
        typedef std::unordered_multimap<int, std::string, HF, Comp, A> C;

        HF hf(42);
        A a(43);
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            }, 12, hf, a);
        assert(c.bucket_count() >= 12);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == HF()));
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
    }
#endif // TEST_STD_VER > 11
}

TEST(init_size)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            },
            7
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >()));
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            },
            7
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));
    }
}

TEST(init_size_hash)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            },
            7,
            test_hash<int>(8)
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >()));
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        C c({
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
            },
            7,
            test_hash<int>(8)
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>());
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));
    }
}

TEST(move)
{
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;

        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<std::pair<const int, std::string> >(10)
        );
        C c = std::move(c0);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 0);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >(10)));
        assert(c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<std::pair<const int, std::string> >(10)
        );
        C::iterator it0 = c0.begin();
        C c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >(10)));

        assert(c0.empty());
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<std::pair<const int, std::string> >()
        );
        C c = std::move(c0);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 0);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<std::pair<const int, std::string> >()
        );
        C::iterator it0 = c0.begin();
        C c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));

        assert(c0.empty());
    }
}

TEST(move_alloc)
{
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(std::move(c0), A(12));
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >(12)));

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A(10)
        );
        C c(std::move(c0), A(10));
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >(10)));

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef min_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A()
        );
        C c(std::move(c0), A());
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef explicit_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_multimap<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            A{}
        );
        C c(std::move(c0), A());
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);

        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::const_iterator i = eq.first;
        assert(i->first == 3);
        assert(i->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        i = eq.first;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A{});

        assert(c0.empty());
    }
}

TEST(size_hash)
{
    {
        typedef std::unordered_multimap<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7,
            test_hash<NotConstructible>(8)
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const NotConstructible, NotConstructible> >()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7,
            test_hash<NotConstructible>(8)
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const NotConstructible, NotConstructible> >()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif

}

TEST(clear)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.size() == 0);
    }
#endif
}

TEST(emplace)
{
    {
        typedef std::unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace(std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace(std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
    {
        typedef std::unordered_multimap<int, Emplaceable, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace(std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace(std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
}

TEST(emplace_hint)
{
    {
        typedef std::unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(3, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
        LIBCPP_ASSERT(r == std::next(c.begin()));

        r = c.emplace_hint(r, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 3);
        assert(r->second == Emplaceable(6, 7));
        LIBCPP_ASSERT(r == std::next(c.begin()));
        r = c.begin();
        assert(r->first == 3);
        LIBCPP_ASSERT(r->second == Emplaceable());
        r = std::next(r, 2);
        assert(r->first == 3);
        LIBCPP_ASSERT(r->second == Emplaceable(5, 6));
    }
    {
        typedef std::unordered_multimap<int, Emplaceable, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(3, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
        LIBCPP_ASSERT(r == std::next(c.begin()));

        r = c.emplace_hint(r, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 3);
        assert(r->second == Emplaceable(6, 7));
        LIBCPP_ASSERT(r == std::next(c.begin()));
        r = c.begin();
        assert(r->first == 3);
        LIBCPP_ASSERT(r->second == Emplaceable());
        r = std::next(r, 2);
        assert(r->first == 3);
        LIBCPP_ASSERT(r->second == Emplaceable(5, 6));
    }
}

TEST(erase_range)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        C::const_iterator i = c.find(2);
        C::const_iterator j = std::next(i, 2);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        k = c.erase(i, j);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        C::const_iterator i = c.find(2);
        C::const_iterator j = std::next(i, 2);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        k = c.erase(i, j);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#endif
}

TEST(insert_hint_rvalue)
{
    {
        typedef std::unordered_multimap<double, int> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5, static_cast<short>(3)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(r, P(3.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(4.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5.5, static_cast<short>(4)));
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(r, P(3, 4));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == 4);

        r = c.insert(c.end(), P(4, 4));
        assert(c.size() == 3);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5, 4));
        assert(c.size() == 4);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_multimap<double, int, std::hash<double>, std::equal_to<double>,
            min_allocator<std::pair<const double, int>>> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5, static_cast<short>(3)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(r, P(3.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(4.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5.5, static_cast<short>(4)));
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
            min_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(r, P(3, 4));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == 4);

        r = c.insert(c.end(), P(4, 4));
        assert(c.size() == 3);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5, 4));
        assert(c.size() == 4);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_multimap<double, int> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, {3.5, 3});
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(r, {3.5, 4});
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        r = c.insert(c.end(), {4.5, 4});
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), {5.5, 4});
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
}

TEST(insert_init)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        C c;
        c.insert(
            {
                P(1, "one"),
                P(2, "two"),
                P(3, "three"),
                P(4, "four"),
                P(1, "four"),
                P(2, "four"),
            }
            );
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::iterator k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        C c;
        c.insert(
            {
                P(1, "one"),
                P(2, "two"),
                P(3, "three"),
                P(4, "four"),
                P(1, "four"),
                P(2, "four"),
            }
            );
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        std::multiset<std::string> s;
        s.insert("one");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(1), c.end(), 1, s);
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        s.insert("two");
        s.insert("four");
        CheckConsecutiveKeys<C::const_iterator>(c.find(2), c.end(), 2, s);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        C::iterator k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
    }
}

TEST(bucket)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(std::begin(a), std::end(a));
        std::size_t bc = c.bucket_count();
        assert(bc >= 7);
        for (std::size_t i = 0; i < 13; ++i)
            LIBCPP_ASSERT(c.bucket(i) == i % bc);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(std::begin(a), std::end(a));
        std::size_t bc = c.bucket_count();
        assert(bc >= 7);
        for (std::size_t i = 0; i < 13; ++i)
            LIBCPP_ASSERT(c.bucket(i) == i % bc);
    }
#endif
}

TEST(bucket_count)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 8);
    }

}

TEST(bucket_size)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 7);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 2);
        LIBCPP_ASSERT(c.bucket_size(2) == 2);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
        LIBCPP_ASSERT(c.bucket_size(5) == 0);
        LIBCPP_ASSERT(c.bucket_size(6) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 7);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 2);
        LIBCPP_ASSERT(c.bucket_size(2) == 2);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
        LIBCPP_ASSERT(c.bucket_size(5) == 0);
        LIBCPP_ASSERT(c.bucket_size(6) == 0);
    }
#endif
}

TEST(count)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fiftyA"),
            P(50, "fiftyB"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.count(30) == 1);
        assert(c.count(50) == 3);
        assert(c.count(5) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fiftyA"),
            P(50, "fiftyB"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.count(30) == 1);
        assert(c.count(50) == 3);
        assert(c.count(5) == 0);
    }
#endif
}

TEST(empty)
{
    {
        typedef std::unordered_multimap<int, double> M;
        M m;
        ASSERT_NOEXCEPT(m.empty());
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, double, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, double>>> M;
        M m;
        ASSERT_NOEXCEPT(m.empty());
        assert(m.empty());
        m.insert(M::value_type(1, 1.5));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#endif
}

TEST(eq)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(testEquality(c1, c2, false));
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert(testEquality(c1, c2, true));
        c2.insert(P(90, "ninety"));
        assert(testEquality(c1, c2, false));
        c1.insert(P(90, "ninety"));
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        assert(testEquality(c1, c2, true));
        c1.insert(P(70, "seventy 2"));
        c2.insert(P(80, "eighty 2"));
        assert(testEquality(c1, c2, false));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(testEquality(c1, c2, false));
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert(testEquality(c1, c2, true));
        c2.insert(P(90, "ninety"));
        assert(testEquality(c1, c2, false));
        c1.insert(P(90, "ninety"));
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        assert(testEquality(c1, c2, true));
        c1.insert(P(70, "seventy 2"));
        c2.insert(P(80, "eighty 2"));
        assert(testEquality(c1, c2, false));
    }
#endif

    // Make sure we take into account the number of times that a key repeats into equality.
    {
        typedef std::pair<int, char> P;
        P a[] = {P(1, 'a'), P(1, 'b'), P(1, 'd'), P(2, 'b')};
        P b[] = {P(1, 'a'), P(1, 'b'), P(1, 'b'), P(1, 'd'), P(2, 'b')};

        std::unordered_multimap<int, char> c1(std::begin(a), std::end(a));
        std::unordered_multimap<int, char> c2(std::begin(b), std::end(b));
        assert(testEquality(c1, c2, false));
    }

    // Make sure we incorporate the values into the equality of the maps.
    // If we were to compare only the keys (including how many time each key repeats),
    // the following test would fail cause only the values differ.
    {
        typedef std::pair<int, char> P;
        P a[] = {P(1, 'a'), P(1, 'b'), P(1, 'd'), P(2, 'b')};
        P b[] = {P(1, 'a'), P(1, 'b'), P(1, 'E'), P(2, 'b')};
        //                                   ^ different here

        std::unordered_multimap<int, char> c1(std::begin(a), std::end(a));
        std::unordered_multimap<int, char> c2(std::begin(b), std::end(b));
        assert(testEquality(c1, c2, false));
    }
}

TEST(key_eq)
{
    typedef std::unordered_multimap<int, std::string> map_type;

    map_type m;
    map_type::iterator i1 = m.insert(map_type::value_type(1, "abc"));
    map_type::iterator i2 = m.insert(map_type::value_type(1, "bcd"));
    map_type::iterator i3 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.key_eq()(i1->first, i1->first));
    assert(cm.key_eq()(i2->first, i2->first));
    assert(cm.key_eq()(i3->first, i3->first));

    assert(cm.key_eq()(i1->first, i2->first));
    assert(cm.key_eq()(i2->first, i1->first));

    assert(!cm.key_eq()(i1->first, i3->first));
    assert(!cm.key_eq()(i3->first, i1->first));

    assert(!cm.key_eq()(i2->first, i3->first));
    assert(!cm.key_eq()(i3->first, i2->first));
}

TEST(load_factor)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#endif
}

TEST(local_iterators)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        typedef C::local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 7);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("one");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 1);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        {
            std::set<std::string> s;
            s.insert("two");
            s.insert("four");
            for (int n = 0; n < 2; ++n) {
                assert(i->first == 2);
                assert(s.find(i->second) != s.end());
                s.erase(s.find(i->second));
                ++i;
            }
        }

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
#endif
}

TEST(max_bucket_count)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#endif
}

TEST(max_load_factor)
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        C c;
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        C c;
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
    }
#endif
}

TEST(size)
{
    {
        typedef std::unordered_multimap<int, double> M;
        M m;
        ASSERT_NOEXCEPT(m.size());
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
        typedef std::unordered_multimap<int, double, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, double>>> M;
        M m;
        ASSERT_NOEXCEPT(m.size());
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
