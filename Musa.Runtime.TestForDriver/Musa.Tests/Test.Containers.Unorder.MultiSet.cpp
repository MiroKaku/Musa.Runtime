#ifdef TEST_HAS_CONTAINERS_UNORDER_MULTI_SET

#include <algorithm>
#include <unordered_set>
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

    template <>
    struct hash<Emplaceable>
    {
        typedef Emplaceable argument_type;
        typedef std::size_t result_type;

        std::size_t operator()(const Emplaceable& x) const
        {
            return x.get();
        }
    };
}

TEST(assign_copy)
{
    {
        typedef test_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
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
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        C* p = &c;
        c = *p;

        assert(c.size() == 6);
        assert(std::is_permutation(c.begin(), c.end(), a));
    }
    {
        typedef other_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
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
        typedef min_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
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
        typedef test_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        C c = {
            P(4),
            P(1),
            P(2)
        };
        c = {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef min_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        C c = {
            P(4),
            P(1),
            P(2)
        };
        c = {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
}

TEST(assign_move)
{
    {
        typedef test_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
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
        typedef test_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid
    }
    {
        typedef other_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid
    }
    {
        typedef test_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid
    }
    {
        typedef min_allocator<int> A;
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            A
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid
    }
}

TEST(copy)
{

    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<int>
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<int>(10)
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == test_allocator<int>(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            other_allocator<int>
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            other_allocator<int>(10)
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == other_allocator<int>(-2));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<int>
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<int>()
        );
        C c = c0;
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        CheckConsecutiveValues<C::const_iterator>(c.find(1), c.end(), 1, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(2), c.end(), 2, 2);
        CheckConsecutiveValues<C::const_iterator>(c.find(3), c.end(), 3, 1);
        CheckConsecutiveValues<C::const_iterator>(c.find(4), c.end(), 4, 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == min_allocator<int>());
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
        typedef std::unordered_multiset<NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<NotConstructible>
        > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() == (test_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<NotConstructible>
        > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() == (min_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef explicit_allocator<NotConstructible> A;
        typedef std::unordered_multiset<NotConstructible,
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
        std::unordered_multiset<int> c = {};
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
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<int>
        > C;
        typedef int P;
        C c = {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == test_allocator<int>());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<int>
        > C;
        typedef int P;
        C c = {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == min_allocator<int>());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER > 11
    {
        typedef int T;
        typedef test_hash<T> HF;
        typedef test_equal_to<T> Comp;
        typedef test_allocator<T> A;
        typedef std::unordered_multiset<T, HF, Comp, A> C;

        A a(42);
        C c({
            T(1),
            T(2),
            T(3),
            T(4),
            T(1),
            T(2)
            }, 12, a);

        assert(c.bucket_count() >= 12);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == HF());
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef int T;
        typedef test_hash<T> HF;
        typedef test_equal_to<T> Comp;
        typedef test_allocator<T> A;
        typedef std::unordered_multiset<T, HF, Comp, A> C;

        A a(42);
        HF hf(43);
        C c({
            T(1),
            T(2),
            T(3),
            T(4),
            T(1),
            T(2)
            }, 12, hf, a);

        assert(c.bucket_count() >= 12);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == HF()));
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif // TEST_STD_VER > 11
}

TEST(init_size)
{
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<int>
        > C;
        typedef int P;
        C c({
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
            },
            7
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == test_allocator<int>());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<int>
        > C;
        typedef int P;
        C c({
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
            },
            7
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == min_allocator<int>());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
}

TEST(move)
{
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<int>
        > C;
        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<int>(10)
        );
        C c = std::move(c0);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 0);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == test_allocator<int>(10));
        assert(c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<int>
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<int>(10)
        );
        C::iterator it0 = c0.begin();
        C c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == test_allocator<int>(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<int>
        > C;
        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<int>()
        );
        C c = std::move(c0);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 0);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == min_allocator<int>());
        assert(c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::unordered_multiset<int,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<int>
        > C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c0(a, a + sizeof(a) / sizeof(a[0]),
            7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<int>()
        );
        C::iterator it0 = c0.begin();
        C c = std::move(c0);
        assert(it0 == c.begin()); // Iterators remain valid
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == min_allocator<int>());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
}

TEST(size)
{

    {
        typedef std::unordered_multiset<NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<NotConstructible>
        > C;
        C c(7);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() == (test_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<NotConstructible>
        > C;
        C c(7);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>());
        assert(c.key_eq() == test_equal_to<NotConstructible>());
        assert(c.get_allocator() == (min_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
}

TEST(bucket)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        const C c(std::begin(a), std::end(a));
        std::size_t bc = c.bucket_count();
        assert(bc >= 7);
        for (std::size_t i = 0; i < 13; ++i)
            LIBCPP_ASSERT(c.bucket(i) == i % bc);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        typedef std::unordered_multiset<int> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 8);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 8);
    }
#endif
}

TEST(bucket_size)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
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

TEST(clear)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a) / sizeof(a[0]));
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.size() == 0);
    }
#endif
}

TEST(count)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.count(30) == 1);
        assert(c.count(50) == 3);
        assert(c.count(5) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(c.count(30) == 1);
        assert(c.count(50) == 3);
        assert(c.count(5) == 0);
    }
#endif
}

TEST(emplace)
{
    {
        typedef std::unordered_multiset<Emplaceable> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace();
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace(Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace(5, 6);
        assert(c.size() == 3);
        assert(*r == Emplaceable(5, 6));
    }
    {
        typedef std::unordered_multiset<Emplaceable, std::hash<Emplaceable>,
            std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace();
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace(Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace(5, 6);
        assert(c.size() == 3);
        assert(*r == Emplaceable(5, 6));
    }
}

TEST(empty)
{
    {
        typedef std::unordered_multiset<int> M;
        M m;
        ASSERT_NOEXCEPT(m.empty());
        assert(m.empty());
        m.insert(M::value_type(1));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> M;
        M m;
        ASSERT_NOEXCEPT(m.empty());
        assert(m.empty());
        m.insert(M::value_type(1));
        assert(!m.empty());
        m.clear();
        assert(m.empty());
    }
#endif
}

TEST(emplace_hint)
{
    {
        typedef std::unordered_multiset<Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e);
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(c.end(), Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 3);
        assert(*r == Emplaceable(5, 6));
    }
    {
        typedef std::unordered_multiset<Emplaceable, std::hash<Emplaceable>,
            std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e);
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(c.end(), Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 3);
        assert(*r == Emplaceable(5, 6));
    }
}

TEST(eq)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert((c1 != c2));
    }
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert((c1 == c2));
        assert(!(c1 != c2));
    }
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert((c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90));
        assert(!(c1 == c2));
        assert((c1 != c2));
        c1.insert(P(90));
        assert((c1 == c2));
        assert(!(c1 != c2));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert((c1 != c2));
    }
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert((c1 == c2));
        assert(!(c1 != c2));
    }
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(20),
            P(30),
            P(40),
            P(50),
            P(50),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert((c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90));
        assert(!(c1 == c2));
        assert((c1 != c2));
        c1.insert(P(90));
        assert((c1 == c2));
        assert(!(c1 != c2));
    }
#endif

    // Make sure we take into account the number of times that a key repeats into equality.
    {
        int a[] = {1, 1, 1, 2};
        int b[] = {1, 1, 1, 1, 2};

        std::unordered_multiset<int> c1(std::begin(a), std::end(a));
        std::unordered_multiset<int> c2(std::begin(b), std::end(b));
        assert(testEquality(c1, c2, false));
    }

    // Make sure we behave properly when a custom key predicate is provided.
    {
        int a[] = {1, 3};
        int b[] = {1, 1};
        // A very poor hash
        struct HashModuloOddness {
            std::size_t operator()(int x) const
            {
                return std::hash<int>()(x % 2);
            }
        };
        // A very poor hash
        struct CompareModuloOddness {
            bool operator()(int x, int y) const
            {
                return (x % 2) == (y % 2);
            }
        };

        using Set = std::unordered_multiset<int, HashModuloOddness, CompareModuloOddness>;
        Set c1(std::begin(a), std::end(a));
        Set c2(std::begin(b), std::end(b));

        assert(testEquality(c1, c2, false));
    }

}

TEST(find_const)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        C::const_iterator i = c.find(30);
        assert(*i == 30);
        i = c.find(5);
        assert(i == c.cend());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        C::const_iterator i = c.find(30);
        assert(*i == 30);
        i = c.find(5);
        assert(i == c.cend());
    }
#endif
}

TEST(insert_hint_rvalue)
{
    {
        typedef std::unordered_multiset<double> C;
        typedef C::iterator R;
        typedef double P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(r, P(3.5));
        assert(c.size() == 2);
        assert(*r == 3.5);

        r = c.insert(c.end(), P(4.5));
        assert(c.size() == 3);
        assert(*r == 4.5);

        r = c.insert(c.end(), P(5.5));
        assert(c.size() == 4);
        assert(*r == 5.5);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<MoveOnly> C;
        typedef C::iterator R;
        typedef MoveOnly P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3));
        assert(c.size() == 1);
        assert(*r == 3);

        r = c.insert(r, P(3));
        assert(c.size() == 2);
        assert(*r == 3);

        r = c.insert(c.end(), P(4));
        assert(c.size() == 3);
        assert(*r == 4);

        r = c.insert(c.end(), P(5));
        assert(c.size() == 4);
        assert(*r == 5);
    }
    {
        typedef std::unordered_multiset<double, std::hash<double>,
            std::equal_to<double>, min_allocator<double>> C;
        typedef C::iterator R;
        typedef double P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(r, P(3.5));
        assert(c.size() == 2);
        assert(*r == 3.5);

        r = c.insert(c.end(), P(4.5));
        assert(c.size() == 3);
        assert(*r == 4.5);

        r = c.insert(c.end(), P(5.5));
        assert(c.size() == 4);
        assert(*r == 5.5);
    }
    {
        typedef std::unordered_multiset<MoveOnly, std::hash<MoveOnly>,
            std::equal_to<MoveOnly>, min_allocator<MoveOnly>> C;
        typedef C::iterator R;
        typedef MoveOnly P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3));
        assert(c.size() == 1);
        assert(*r == 3);

        r = c.insert(r, P(3));
        assert(c.size() == 2);
        assert(*r == 3);

        r = c.insert(c.end(), P(4));
        assert(c.size() == 3);
        assert(*r == 4);

        r = c.insert(c.end(), P(5));
        assert(c.size() == 4);
        assert(*r == 5);
    }
#endif // TEST_STD_VER >= 11
}

TEST(insert_init)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        C c;
        c.insert(
            {
                P(1),
                P(2),
                P(3),
                P(4),
                P(1),
                P(2)
            }
        );
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
    }
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        C c;
        c.insert(
            {
                P(1),
                P(2),
                P(3),
                P(4),
                P(1),
                P(2)
            }
        );
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);
    }
}

TEST(insert_rvalue)
{

    {
        typedef std::unordered_multiset<double> C;
        typedef C::iterator R;
        typedef double P;
        C c;
        R r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(P(3.5));
        assert(c.size() == 2);
        assert(*r == 3.5);

        r = c.insert(P(4.5));
        assert(c.size() == 3);
        assert(*r == 4.5);

        r = c.insert(P(5.5));
        assert(c.size() == 4);
        assert(*r == 5.5);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<MoveOnly> C;
        typedef C::iterator R;
        typedef MoveOnly P;
        C c;
        R r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r == 3);

        r = c.insert(P(3));
        assert(c.size() == 2);
        assert(*r == 3);

        r = c.insert(P(4));
        assert(c.size() == 3);
        assert(*r == 4);

        r = c.insert(P(5));
        assert(c.size() == 4);
        assert(*r == 5);
    }
    {
        typedef std::unordered_multiset<double, std::hash<double>,
            std::equal_to<double>, min_allocator<double>> C;
        typedef C::iterator R;
        typedef double P;
        C c;
        R r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(P(3.5));
        assert(c.size() == 2);
        assert(*r == 3.5);

        r = c.insert(P(4.5));
        assert(c.size() == 3);
        assert(*r == 4.5);

        r = c.insert(P(5.5));
        assert(c.size() == 4);
        assert(*r == 5.5);
    }
    {
        typedef std::unordered_multiset<MoveOnly, std::hash<MoveOnly>,
            std::equal_to<MoveOnly>, min_allocator<MoveOnly>> C;
        typedef C::iterator R;
        typedef MoveOnly P;
        C c;
        R r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r == 3);

        r = c.insert(P(3));
        assert(c.size() == 2);
        assert(*r == 3);

        r = c.insert(P(4));
        assert(c.size() == 3);
        assert(*r == 4);

        r = c.insert(P(5));
        assert(c.size() == 4);
        assert(*r == 5);
    }
#endif // TEST_STD_VER >= 11
}

TEST(key_eq)
{
    typedef std::unordered_multiset<int> set_type;
    set_type s;

    set_type::iterator i1 = s.insert(1);
    set_type::iterator i2 = s.insert(1);
    set_type::iterator i3 = s.insert(2);

    const set_type& cs = s;

    assert(cs.key_eq()(*i1, *i1));
    assert(cs.key_eq()(*i2, *i2));
    assert(cs.key_eq()(*i3, *i3));

    assert(cs.key_eq()(*i1, *i2));
    assert(cs.key_eq()(*i2, *i1));

    assert(!cs.key_eq()(*i1, *i3));
    assert(!cs.key_eq()(*i3, *i1));

    assert(!cs.key_eq()(*i2, *i3));
    assert(!cs.key_eq()(*i3, *i2));

}

TEST(load_factor)
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_multiset<int> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#endif
}

TEST(max_bucket_count)
{
    {
        typedef std::unordered_multiset<int> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
            std::equal_to<int>, min_allocator<int>> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#endif
}

TEST(size)
{
    {
        typedef std::unordered_multiset<int> M;
        M m;
        ASSERT_NOEXCEPT(m.size());
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
        typedef std::unordered_multiset<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> M;
        M m;
        ASSERT_NOEXCEPT(m.size());
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


#endif
