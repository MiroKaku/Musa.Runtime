#ifdef TEST_HAS_CONTAINERS_UNORDER_MAP

#include <algorithm>
#include <unordered_map>
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.size() == 4);
        assert(std::is_permutation(c.begin(), c.end(), a));
    }
    {
        typedef other_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c0.size() == 0);
        assert(it0 == c.begin()); // Iterators remain valid
    }
    {
        typedef other_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c0.size() == 0);
        assert(it0 == c.begin()); // Iterators remain valid
    }
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef std::unordered_map<int, std::string,
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(c0.size() == 0);
        assert(it0 == c.begin()); // Iterators remain valid
    }
}

TEST(copy)
{
    {
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
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
        std::unordered_map<int, int> c = {};
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER > 11
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef test_hash<int> HF;
        typedef test_equal_to<int> Comp;
        typedef std::unordered_map<int, std::string, HF, Comp, A> C;

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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == a);
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef test_hash<int> HF;
        typedef test_equal_to<int> Comp;
        typedef std::unordered_map<int, std::string, HF, Comp, A> C;

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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == test_hash<int>()));
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() == a);
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
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>());
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
}

TEST(init_size_hash)
{
    {
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>());
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
}

TEST(init_size_hash_equal)
{
    {
        typedef std::unordered_map<int, std::string,
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
            test_hash<int>(8),
            test_equal_to<int>(9)
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string,
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
            test_hash<int>(8),
            test_equal_to<int>(9)
            );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
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
}

TEST(move)
{
    {
        typedef std::unordered_map<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            test_allocator<std::pair<const int, std::string> >
        > C;
        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            test_allocator<std::pair<const int, std::string> >(10)
        );
        C::iterator it0 = c0.begin();
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
        assert(it0 == c.begin()); // Iterators remain valid

        assert(c0.empty());
    }
    {
        typedef std::unordered_map<int, std::string,
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (test_allocator<std::pair<const int, std::string> >(10)));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid

        assert(c0.empty());
    }
    {
        typedef std::unordered_map<int, std::string,
            test_hash<int>,
            test_equal_to<int>,
            min_allocator<std::pair<const int, std::string> >
        > C;
        C c0(7,
            test_hash<int>(8),
            test_equal_to<int>(9),
            min_allocator<std::pair<const int, std::string> >()
        );
        C::iterator it0 = c0.begin();
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
        assert(it0 == c.begin()); // Iterators remain valid

        assert(c0.empty());
    }
    {
        typedef std::unordered_map<int, std::string,
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() ==
            (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
        assert(it0 == c.begin()); // Iterators remain valid

        assert(c0.empty());
    }
}

TEST(move_alloc)
{
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(12));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef test_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef min_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_map<int, std::string,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        assert(std::fabs(c.load_factor() - (float)c.size() / c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);

        assert(c0.empty());
    }
    {
        typedef std::pair<int, std::string> P;
        typedef explicit_allocator<std::pair<const int, std::string>> A;
        typedef std::unordered_map<int, std::string,
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
        C c(std::move(c0), A{});
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<int>(8));
        assert(c.key_eq() == test_equal_to<int>(9));
        assert(c.get_allocator() == A{});
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7);
        LIBCPP_ASSERT(c.bucket_count() == 7);
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7);
        LIBCPP_ASSERT(c.bucket_count() == 7);
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
#endif
}

TEST(size_hash)
{
    {
        typedef std::unordered_map<NotConstructible, NotConstructible,
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
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

TEST(size_hash_equal)
{
    {
        typedef std::unordered_map<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            test_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7,
            test_hash<NotConstructible>(8),
            test_equal_to<NotConstructible>(9)
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>(9));
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
            test_hash<NotConstructible>,
            test_equal_to<NotConstructible>,
            min_allocator<std::pair<const NotConstructible,
            NotConstructible> >
        > C;
        C c(7,
            test_hash<NotConstructible>(8),
            test_equal_to<NotConstructible>(9)
        );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>(9));
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

TEST(at)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.size() == 4);
        c.at(1) = "ONE";
        assert(c.at(1) == "ONE");
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            c.at(11) = "eleven";
            assert(false);
        }
        catch (std::out_of_range&) {
        }
        assert(c.size() == 4);
    #endif
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.size() == 4);
        assert(c.at(1) == "one");
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD c.at(11);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
        assert(c.size() == 4);
    #endif
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.size() == 4);
        c.at(1) = "ONE";
        assert(c.at(1) == "ONE");
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            c.at(11) = "eleven";
            assert(false);
        }
        catch (std::out_of_range&) {
        }
        assert(c.size() == 4);
    #endif
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.size() == 4);
        assert(c.at(1) == "one");
    #ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD c.at(11);
            assert(false);
        }
        catch (std::out_of_range&) {
        }
        assert(c.size() == 4);
    #endif
    }
#endif
}

TEST(clear)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        typedef std::unordered_map<int, Emplaceable> C;
        typedef std::pair<C::iterator, bool> R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(r.second);
        assert(c.size() == 1);
        assert(r.first->first == 3);
        assert(r.first->second == Emplaceable());

        r = c.emplace(std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(r.second);
        assert(c.size() == 2);
        assert(r.first->first == 4);
        assert(r.first->second == Emplaceable(5, 6));

        r = c.emplace(std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(r.second);
        assert(c.size() == 3);
        assert(r.first->first == 5);
        assert(r.first->second == Emplaceable(6, 7));
    }
    {
        typedef std::unordered_map<int, Emplaceable, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef std::pair<C::iterator, bool> R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(r.second);
        assert(c.size() == 1);
        assert(r.first->first == 3);
        assert(r.first->second == Emplaceable());

        r = c.emplace(std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(r.second);
        assert(c.size() == 2);
        assert(r.first->first == 4);
        assert(r.first->second == Emplaceable(5, 6));

        r = c.emplace(std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(r.second);
        assert(c.size() == 3);
        assert(r.first->first == 5);
        assert(r.first->second == Emplaceable(6, 7));
    }
}

TEST(emplace_hint)
{
    {
        typedef std::unordered_map<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
    {
        typedef std::unordered_map<int, Emplaceable, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
            std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(5),
            std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
}

namespace
{
    namespace erase_key
    {
    #if TEST_STD_VER >= 11
        template <typename Unordered>
        bool only_deletions(const Unordered& whole, const Unordered& part)
        {
            typename Unordered::const_iterator w = whole.begin();
            typename Unordered::const_iterator p = part.begin();

            while (w != whole.end() && p != part.end()) {
                if (*w == *p)
                    p++;
                w++;
            }

            return p == part.end();
        }
    #endif
    }
}

TEST(erase_key)
{
    using namespace erase_key;

    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.erase(5) == 0);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 1);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 0);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(4) == 1);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(4) == 0);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(1) == 1);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.erase(5) == 0);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 1);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 0);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(4) == 1);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(4) == 0);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(1) == 1);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
    {
        typedef std::unordered_map<int, int> C;
        C m, m2;
        for (int i = 0; i < 10; ++i) {
            m[i] = i;
            m2[i] = i;
        }

        C::iterator i = m2.begin();
        int ctr = 0;
        while (i != m2.end()) {
            if (ctr++ % 2 == 0)
                m2.erase(i++);
            else
                ++i;
        }

        assert(only_deletions(m, m2));
    }
#endif
}

TEST(erase_range)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        C::const_iterator j = std::next(i, 1);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        k = c.erase(i, j);
        assert(c.size() == 3);
        assert(k == j);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        k = c.erase(c.cbegin(), c.cend());
        assert(k == c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        C::const_iterator j = std::next(i, 1);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        k = c.erase(i, j);
        assert(c.size() == 3);
        assert(k == j);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        k = c.erase(c.cbegin(), c.cend());
        assert(k == c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#endif
}

namespace
{
    namespace extract_key
    {
        template <class Container, class KeyTypeIter>
        void test(Container& c, KeyTypeIter first, KeyTypeIter last)
        {
            std::size_t sz = c.size();
            assert((std::size_t)std::distance(first, last) == sz);

            for (KeyTypeIter copy = first; copy != last; ++copy) {
                typename Container::node_type t = c.extract(*copy);
                assert(!t.empty());
                --sz;
                assert(t.key() == *copy);
                t.key() = *first; // We should be able to mutate key.
                assert(t.key() == *first);
                assert(t.get_allocator() == c.get_allocator());
                assert(sz == c.size());
            }

            assert(c.size() == 0);

            for (KeyTypeIter copy = first; copy != last; ++copy) {
                typename Container::node_type t = c.extract(*copy);
                assert(t.empty());
            }
        }

    }
}

TEST(extract_key)
{
    using namespace extract_key;

    {
        std::unordered_map<int, int> m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }

    {
        std::unordered_map<Counter<int>, Counter<int>> m =
        {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        {
            Counter<int> keys[] = {1, 2, 3, 4, 5, 6};
            assert(Counter_base::gConstructed == 12 + 6);
            test(m, std::begin(keys), std::end(keys));
        }
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_map =
            std::unordered_map<int, int, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }
}

namespace
{
    namespace insert_const_lvalue
    {
        template <class Container>
        void do_insert_cv_test()
        {
            typedef Container M;
            typedef std::pair<typename M::iterator, bool> R;
            typedef typename M::value_type VT;
            M m;

            const VT v1(2.5, 2);
            R r = m.insert(v1);
            assert(r.second);
            assert(m.size() == 1);
            assert(r.first->first == 2.5);
            assert(r.first->second == 2);

            const VT v2(2.5, 3);
            r = m.insert(v2);
            assert(!r.second);
            assert(m.size() == 1);
            assert(r.first->first == 2.5);
            assert(r.first->second == 2);

            const VT v3(1.5, 1);
            r = m.insert(v3);
            assert(r.second);
            assert(m.size() == 2);
            assert(r.first->first == 1.5);
            assert(r.first->second == 1);

            const VT v4(3.5, 3);
            r = m.insert(v4);
            assert(r.second);
            assert(m.size() == 3);
            assert(r.first->first == 3.5);
            assert(r.first->second == 3);

            const VT v5(3.5, 4);
            r = m.insert(v5);
            assert(!r.second);
            assert(m.size() == 3);
            assert(r.first->first == 3.5);
            assert(r.first->second == 3);
        }
    }
}

TEST(insert_const_lvalue)
{
    using namespace insert_const_lvalue;

    {
        typedef std::unordered_map<double, int> M;
        do_insert_cv_test<M>();
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
            min_allocator<std::pair<const double, int>>> M;
        do_insert_cv_test<M>();
    }
#endif
}

namespace
{
    namespace insert_hint_const_lvalue
    {
        template<class Container>
        void do_insert_hint_const_lvalue_test()
        {
            typedef Container C;
            typedef typename C::iterator R;
            typedef typename C::value_type VT;
            C c;
            typename C::const_iterator e = c.end();
            const VT v1(3.5, 3);
            R r = c.insert(e, v1);
            assert(c.size() == 1);
            assert(r->first == 3.5);
            assert(r->second == 3);

            const VT v2(3.5, 4);
            r = c.insert(r, v2);
            assert(c.size() == 1);
            assert(r->first == 3.5);
            assert(r->second == 3);

            const VT v3(4.5, 4);
            r = c.insert(c.end(), v3);
            assert(c.size() == 2);
            assert(r->first == 4.5);
            assert(r->second == 4);

            const VT v4(5.5, 4);
            r = c.insert(c.end(), v4);
            assert(c.size() == 3);
            assert(r->first == 5.5);
            assert(r->second == 4);
        }

    }
}

TEST(insert_hint_const_lvalue)
{
    using namespace insert_hint_const_lvalue;

    do_insert_hint_const_lvalue_test<std::unordered_map<double, int> >();
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
            min_allocator<std::pair<const double, int>>> C;

        do_insert_hint_const_lvalue_test<C>();
    }
#endif
}

TEST(insert_hint_rvalue)
{
    {
        typedef std::unordered_map<double, int> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5, static_cast<short>(3)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(r, P(3.5, static_cast<short>(4)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(c.end(), P(4.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<MoveOnly, MoveOnly> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(r, P(3, 4));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(c.end(), P(4, 4));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5, 4));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
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
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(c.end(), P(4.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<MoveOnly, MoveOnly, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
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
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(c.end(), P(4, 4));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(c.end(), P(5, 4));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<double, MoveOnly> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, {3.5, 3});
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(r, {3.5, 4});
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(c.end(), {4.5, 4});
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(c.end(), {5.5, 4});
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }

}

TEST(insert_init)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
    }
}

TEST(bucket)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(bc >= 5);
        for (std::size_t i = 0; i < 13; ++i)
            LIBCPP_ASSERT(c.bucket(i) == i % bc);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(bc >= 5);
        for (std::size_t i = 0; i < 13; ++i)
            LIBCPP_ASSERT(c.bucket(i) == i % bc);
    }
#endif
}

TEST(bucket_count)
{
    {
        typedef std::unordered_map<int, std::string> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 8);
    }
#endif
}

TEST(bucket_size)
{

    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 1);
        LIBCPP_ASSERT(c.bucket_size(2) == 1);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 1);
        LIBCPP_ASSERT(c.bucket_size(2) == 1);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
    }
#endif
}

namespace
{
    namespace compare
    {
        struct Key {
            template <typename T> Key(const T&)
            {}
            bool operator== (const Key&) const
            {
                return true;
            }
        };
    }
}

namespace std
{
    template <>
    struct hash<compare::Key>
    {
        std::size_t operator()(compare::Key const&) const
        {
            return 0;
        }
    };
}

TEST(compare)
{
    using namespace compare;

    typedef std::unordered_map<Key, int> MapT;
    typedef MapT::iterator Iter;
    MapT map;
    Iter it = map.find(Key(0));
    assert(it == map.end());
    std::pair<Iter, bool> result = map.insert(std::make_pair(Key(0), 42));
    assert(result.second);
    assert(result.first->second == 42);
}

namespace
{
    namespace contains
    {
        template <typename T, typename P, typename B, typename... Pairs>
        void test(B bad, Pairs... args)
        {
            T map;
            P pairs[] = {args...};

            for (auto& p : pairs) map.insert(p);
            for (auto& p : pairs) assert(map.contains(p.first));

            assert(!map.contains(bad));
        }

        struct E {
            int a = 1; double b = 1; char c = 1;
        };
    }
}

TEST(contains)
{
    using namespace contains;

    {
        test<std::unordered_map<char, int>, std::pair<char, int> >(
            'e', std::make_pair('a', 10), std::make_pair('b', 11),
            std::make_pair('c', 12), std::make_pair('d', 13));

        test<std::unordered_map<char, char>, std::pair<char, char> >(
            'e', std::make_pair('a', 'a'), std::make_pair('b', 'a'),
            std::make_pair('c', 'a'), std::make_pair('d', 'b'));

        test<std::unordered_map<int, E>, std::pair<int, E> >(
            -1, std::make_pair(1, E{}), std::make_pair(2, E{}),
            std::make_pair(3, E{}), std::make_pair(4, E{}));
    }
    {
        test<std::unordered_multimap<char, int>, std::pair<char, int> >(
            'e', std::make_pair('a', 10), std::make_pair('b', 11),
            std::make_pair('c', 12), std::make_pair('d', 13));

        test<std::unordered_multimap<char, char>, std::pair<char, char> >(
            'e', std::make_pair('a', 'a'), std::make_pair('b', 'a'),
            std::make_pair('c', 'a'), std::make_pair('d', 'b'));

        test<std::unordered_multimap<int, E>, std::pair<int, E> >(
            -1, std::make_pair(1, E{}), std::make_pair(2, E{}),
            std::make_pair(3, E{}), std::make_pair(4, E{}));
    }
}

TEST(contains_transparent)
{
    using key_type = StoredType<int>;

    {
        // Make sure conversions don't happen for transparent non-final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<>>;
        test_transparent_contains<M>({{1, 2}, {2, 3}});
        test_transparent_contains<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions don't happen for transparent final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash_final, transparent_equal_final>;
        test_transparent_contains<M>({{1, 2}, {2, 3}});
        test_transparent_contains<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent hasher
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<>>;
        test_non_transparent_contains<M>({{1, 2}, {2, 3}});
        test_non_transparent_contains<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_contains<M>({{1, 2}, {2, 3}});
        test_non_transparent_contains<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for both non-transparent hasher and key_equal
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_contains<M>({{1, 2}, {2, 3}});
        test_non_transparent_contains<const M>({{1, 2}, {2, 3}});
    }
}

TEST(count)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.count(30) == 1);
        assert(c.count(5) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.count(30) == 1);
        assert(c.count(5) == 0);
    }
#endif

}

TEST(count_transparent)
{
    using key_type = StoredType<int>;

    {
        // Make sure conversions don't happen for transparent non-final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<>>;
        test_transparent_count<M>({{1, 2}, {2, 3}});
        test_transparent_count<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions don't happen for transparent final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash_final, transparent_equal_final>;
        test_transparent_count<M>({{1, 2}, {2, 3}});
        test_transparent_count<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent hasher
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<>>;
        test_non_transparent_count<M>({{1, 2}, {2, 3}});
        test_non_transparent_count<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_count<M>({{1, 2}, {2, 3}});
        test_non_transparent_count<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for both non-transparent hasher and key_equal
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_count<M>({{1, 2}, {2, 3}});
        test_non_transparent_count<const M>({{1, 2}, {2, 3}});
    }
}

TEST(empty)
{
    {
        typedef std::unordered_map<int, double> M;
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
        typedef std::unordered_map<int, double, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, double>>> M;
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
        typedef std::unordered_map<int, std::string> C;
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
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(testEquality(c1, c2, false));
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        typedef std::unordered_map<int, std::string> C;
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
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        assert(testEquality(c1, c2, true));
        c1.insert(P(90, "ninety"));
        c2.insert(P(100, "onehundred"));
        assert(testEquality(c1, c2, false));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(testEquality(c1, c2, false));
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert(testEquality(c1, c2, true));
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        assert(testEquality(c1, c2, true));
        c1.insert(P(90, "ninety"));
        c2.insert(P(100, "onehundred"));
        assert(testEquality(c1, c2, false));
    }
#endif
}

TEST(equal_range_transparent)
{
    using key_type = StoredType<int>;

    {
        // Make sure conversions don't happen for transparent non-final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<>>;
        test_transparent_equal_range<M>({{1, 2}, {2, 3}});
        test_transparent_equal_range<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions don't happen for transparent final hasher and key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash_final, transparent_equal_final>;
        test_transparent_equal_range<M>({{1, 2}, {2, 3}});
        test_transparent_equal_range<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent hasher
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<>>;
        test_non_transparent_equal_range<M>({{1, 2}, {2, 3}});
        test_non_transparent_equal_range<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for non-transparent key_equal
        using M = unord_map_type<std::unordered_map, transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_equal_range<M>({{1, 2}, {2, 3}});
        test_non_transparent_equal_range<const M>({{1, 2}, {2, 3}});
    }

    {
        // Make sure conversions do happen for both non-transparent hasher and key_equal
        using M = unord_map_type<std::unordered_map, non_transparent_hash, std::equal_to<key_type>>;
        test_non_transparent_equal_range<M>({{1, 2}, {2, 3}});
        test_non_transparent_equal_range<const M>({{1, 2}, {2, 3}});
    }
}

TEST(equal_range_const)
{
    {
        typedef std::unordered_map<int, std::string> C;
        typedef C::const_iterator I;
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
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef C::const_iterator I;
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
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
    }
#endif
}

TEST(equal_range_non_const)
{
    {
        typedef std::unordered_map<int, std::string> C;
        typedef C::iterator I;
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
        C c(std::begin(a), std::end(a));
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        typedef C::iterator I;
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
        C c(std::begin(a), std::end(a));
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
    }
#endif
}

TEST(find_const)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        C::const_iterator i = c.find(30);
        assert(i->first == 30);
        assert(i->second == "thirty");
        i = c.find(5);
        assert(i == c.cend());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        C::const_iterator i = c.find(30);
        assert(i->first == 30);
        assert(i->second == "thirty");
        i = c.find(5);
        assert(i == c.cend());
    }
#endif
}

TEST(iterators)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        C::iterator i;
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        C::const_iterator i;
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        C::iterator i;
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        const C c(a, a + sizeof(a) / sizeof(a[0]));
        assert(c.bucket_count() >= 5);
        assert(c.size() == 4);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
        C::const_iterator i;
    }
#endif
#if TEST_STD_VER > 11
    { // N3644 testing
        typedef std::unordered_map<int, double> C;
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

TEST(key_eq)
{
    typedef std::unordered_map<int, std::string> map_type;

    map_type m;
    std::pair<map_type::iterator, bool> p1 = m.insert(map_type::value_type(1, "abc"));
    std::pair<map_type::iterator, bool> p2 = m.insert(map_type::value_type(2, "abc"));

    const map_type& cm = m;

    assert(cm.key_eq()(p1.first->first, p1.first->first));
    assert(cm.key_eq()(p2.first->first, p2.first->first));
    assert(!cm.key_eq()(p1.first->first, p2.first->first));
    assert(!cm.key_eq()(p2.first->first, p1.first->first));
}

TEST(load_factor)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        typedef std::unordered_map<int, std::string> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#endif
}

TEST(local_iterators)
{
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.bucket_count() >= 5);
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 1);
        assert(i->second == "one");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 2);
        assert(i->second == "two");

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
    }
#endif
}

TEST(max_bucket_count)
{
    {
        typedef std::unordered_map<int, std::string> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#endif
}

TEST(max_load_factor)
{
    {
        typedef std::unordered_map<int, std::string> C;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string> C;
        C c;
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        typedef std::unordered_map<int, double> M;
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
        typedef std::unordered_map<int, double, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, double>>> M;
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

TEST(swap_member)
{
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef test_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef test_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a2[] =
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
        C c1(0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef test_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef test_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        P a2[] =
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
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }

    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef other_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc(1));
        C c2(0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef other_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a2[] =
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
        C c1(0, Hash(1), Compare(1), Alloc(1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef other_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1));
        C c2(0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef other_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        P a2[] =
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
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef min_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc());
        C c2(0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef min_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a2[] =
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
        C c1(0, Hash(1), Compare(1), Alloc());
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef min_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc());
        C c2(0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<int> Hash;
        typedef test_equal_to<int> Compare;
        typedef min_allocator<std::pair<const int, std::string> > Alloc;
        typedef std::unordered_map<int, std::string, Hash, Compare, Alloc> C;
        typedef std::pair<int, std::string> P;
        P a1[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        P a2[] =
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
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc());
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(c1.at(10) == "ten");
        assert(c1.at(20) == "twenty");
        assert(c1.at(30) == "thirty");
        assert(c1.at(40) == "forty");
        assert(c1.at(50) == "fifty");
        assert(c1.at(60) == "sixty");
        assert(c1.at(70) == "seventy");
        assert(c1.at(80) == "eighty");
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.at(1) == "one");
        assert(c2.at(2) == "two");
        assert(c2.at(3) == "three");
        assert(c2.at(4) == "four");
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
#endif
}


#endif
