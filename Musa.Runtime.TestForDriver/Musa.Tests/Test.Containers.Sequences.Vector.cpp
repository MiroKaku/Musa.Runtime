#ifdef TEST_HAS_CONTAINERS_VECTOR

#include <vector>
#include <stdexcept>
#include <cassert>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/test_allocator.h"
#include <LLVM.TestSuite/test_comparisons.h>
#include "LLVM.TestSuite/min_allocator.h"
#include "LLVM.TestSuite/asan_testing.h"
#include "LLVM.TestSuite/MoveOnly.h"
#include "LLVM.TestSuite/DefaultOnly.h"
#include "LLVM.TestSuite/allocators.h"


TEST(capacity)
{
    {
        std::vector<int> v;
        assert(v.capacity() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v;
        assert(v.capacity() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(test_empty)
{
    {
        typedef std::vector<int> C;
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
        typedef std::vector<int, min_allocator<int>> C;
        C c;
        ASSERT_NOEXCEPT(c.empty());
        assert(c.empty());
        c.push_back(C::value_type(1));
        assert(!c.empty());
        c.clear();
        assert(c.empty());
    }
    {
        typedef std::vector<int, safe_allocator<int>> C;
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

TEST(reserve)
{
    {
        std::vector<int> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 250 + 1> > v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        std::vector<int> v;
        std::size_t sz = v.max_size() + 1;

        try {
            v.reserve(sz);
            assert(false);
        }
        catch (const std::length_error&) {
            assert(v.size() == 0);
            assert(v.capacity() == 0);
        }
    }
    if (!TEST_IS_CONSTANT_EVALUATED) {
        std::vector<int> v(10, 42);
        int* previous_data = v.data();
        std::size_t previous_capacity = v.capacity();
        std::size_t sz = v.max_size() + 1;

        try {
            v.reserve(sz);
            assert(false);
        }
        catch (std::length_error&) {
            assert(v.size() == 10);
            assert(v.capacity() == previous_capacity);
            assert(v.data() == previous_data);

            for (int i = 0; i < 10; ++i) {
                assert(v[i] == 42);
            }
        }
    }
#endif
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        std::vector<int, limited_allocator<int, 100> > v;
        v.reserve(50);
        assert(v.capacity() == 50);
        assert(is_contiguous_container_asan_correct(v));
        try {
            v.reserve(101);
            assert(false);
        }
        catch (const std::length_error&) {
            // no-op
        }
        assert(v.capacity() == 50);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(resize_size)
{
    {
        std::vector<int> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 300 + 1> > v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<MoveOnly> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<MoveOnly, limited_allocator<MoveOnly, 300 + 1> > v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<MoveOnly, min_allocator<MoveOnly>> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(resize_size_value)
{
    {
        std::vector<int> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(v == std::vector<int>(50));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 300 + 1> > v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        assert((v == std::vector<int, min_allocator<int>>(50)));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        assert((v == std::vector<int, safe_allocator<int>>(50)));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(shrink_to_fit)
{
    {
        std::vector<int> v(100);
        v.push_back(1);
        assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, limited_allocator<int, 401> > v(100);
        v.push_back(1);
        assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        std::vector<int, limited_allocator<int, 400> > v(100);
        v.push_back(1);
        assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        LIBCPP_ASSERT(v.capacity() == 200); // assumes libc++'s 2x growth factor
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v(100);
        v.push_back(1);
        assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        v.push_back(1);
        assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(size)
{
    {
        typedef std::vector<int> C;
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
        typedef std::vector<int, min_allocator<int>> C;
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
    {
        typedef std::vector<int, safe_allocator<int>> C;
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

TEST(swap)
{
    {
        std::vector<int> v1(100);
        std::vector<int> v2(200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v2));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v1(100);
        std::vector<int, min_allocator<int>> v2(200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v2));
    }
    {
        std::vector<int, safe_allocator<int>> v1(100);
        std::vector<int, safe_allocator<int>> v2(200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v2));
    }
#endif
}

TEST(assign_copy)
{
    {
        std::vector<int, test_allocator<int> > l(3, 2, test_allocator<int>(5));
        std::vector<int, test_allocator<int> > l2(l, test_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<int>(3));
    }
    {
        std::vector<int, other_allocator<int> > l(3, 2, other_allocator<int>(5));
        std::vector<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(5));
    }
#if TEST_STD_VER >= 11
    {
        // Test with Allocator::propagate_on_container_copy_assignment == false_type
        using Alloc = NonPOCCAAllocator<int>;
        bool copy_assigned_into = false;
        std::vector<int, Alloc> l(3, 2, Alloc(5, nullptr));
        std::vector<int, Alloc> l2(l, Alloc(3, &copy_assigned_into));
        assert(!copy_assigned_into);
        l2 = l;
        assert(!copy_assigned_into);
        assert(l2 == l);
        assert(l2.get_allocator() == Alloc(3, nullptr));
    }
    {
        // Test with Allocator::propagate_on_container_copy_assignment == true_type
        // and equal allocators
        using Alloc = POCCAAllocator<int>;
        bool copy_assigned_into = false;
        std::vector<int, Alloc> l(3, 2, Alloc(5, nullptr));
        std::vector<int, Alloc> l2(l, Alloc(5, &copy_assigned_into));
        assert(!copy_assigned_into);
        l2 = l;
        assert(copy_assigned_into);
        assert(l2 == l);
        assert(l2.get_allocator() == Alloc(5, nullptr));
    }
    {
        // Test with Allocator::propagate_on_container_copy_assignment == true_type
        // and unequal allocators
        using Alloc = POCCAAllocator<int>;
        bool copy_assigned_into = false;
        std::vector<int, Alloc> l(3, 2, Alloc(5, nullptr));
        std::vector<int, Alloc> l2(l, Alloc(3, &copy_assigned_into));
        assert(!copy_assigned_into);
        l2 = l;
        assert(copy_assigned_into);
        assert(l2 == l);
        assert(l2.get_allocator() == Alloc(5, nullptr));
    }
    {
        std::vector<int, min_allocator<int> > l(3, 2, min_allocator<int>());
        std::vector<int, min_allocator<int> > l2(l, min_allocator<int>());
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
    {
        std::vector<int, safe_allocator<int> > l(3, 2, safe_allocator<int>());
        std::vector<int, safe_allocator<int> > l2(l, safe_allocator<int>());
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == safe_allocator<int>());
    }
#endif
}

TEST(assign_move)
{
    {
        std::vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(5));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(6));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(6));
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::vector<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, other_allocator<MoveOnly> > l2(other_allocator<MoveOnly>(6));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, min_allocator<MoveOnly> > l((min_allocator<MoveOnly>()));
        std::vector<MoveOnly, min_allocator<MoveOnly> > lo((min_allocator<MoveOnly>()));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, min_allocator<MoveOnly> > l2((min_allocator<MoveOnly>()));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, safe_allocator<MoveOnly> > l((safe_allocator<MoveOnly>()));
        std::vector<MoveOnly, safe_allocator<MoveOnly> > lo((safe_allocator<MoveOnly>()));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, safe_allocator<MoveOnly> > l2((safe_allocator<MoveOnly>()));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
}

TEST(move)
{
    test_allocator_statistics alloc_stats;
    {
        std::vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5, &alloc_stats));
        std::vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5, &alloc_stats));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, test_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::vector<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, other_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        std::vector<int> c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        assert(is_contiguous_container_asan_correct(c1));
        std::vector<int>::const_iterator i = c1.begin();
        std::vector<int> c2 = std::move(c1);
        assert(is_contiguous_container_asan_correct(c2));
        std::vector<int>::iterator j = c2.erase(i);
        assert(*j == 3);
        assert(is_contiguous_container_asan_correct(c2));
    }
    {
        std::vector<MoveOnly, min_allocator<MoveOnly> > l((min_allocator<MoveOnly>()));
        std::vector<MoveOnly, min_allocator<MoveOnly> > lo((min_allocator<MoveOnly>()));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, min_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        std::vector<int, min_allocator<int> > c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        assert(is_contiguous_container_asan_correct(c1));
        std::vector<int, min_allocator<int> >::const_iterator i = c1.begin();
        std::vector<int, min_allocator<int> > c2 = std::move(c1);
        assert(is_contiguous_container_asan_correct(c2));
        std::vector<int, min_allocator<int> >::iterator j = c2.erase(i);
        assert(*j == 3);
        assert(is_contiguous_container_asan_correct(c2));
    }
    {
        std::vector<MoveOnly, safe_allocator<MoveOnly> > l((safe_allocator<MoveOnly>()));
        std::vector<MoveOnly, safe_allocator<MoveOnly> > lo((safe_allocator<MoveOnly>()));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i) {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, safe_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        std::vector<int, safe_allocator<int> > c1(a1, a1 + sizeof(a1) / sizeof(a1[0]));
        assert(is_contiguous_container_asan_correct(c1));
        std::vector<int, safe_allocator<int> >::const_iterator i = c1.begin();
        std::vector<int, safe_allocator<int> > c2 = std::move(c1);
        assert(is_contiguous_container_asan_correct(c2));
        std::vector<int, safe_allocator<int> >::iterator j = c2.erase(i);
        assert(*j == 3);
        assert(is_contiguous_container_asan_correct(c2));
    }
    {
        alloc_stats.clear();
        using Vect = std::vector<int, test_allocator<int> >;
        Vect v(test_allocator<int>(42, 101, &alloc_stats));
        assert(alloc_stats.count == 1);
        assert(alloc_stats.copied == 1);
        assert(alloc_stats.moved == 0);
        {
            const test_allocator<int>& a = v.get_allocator();
            assert(a.get_data() == 42);
            assert(a.get_id() == 101);
        }
        assert(alloc_stats.count == 1);
        alloc_stats.clear_ctor_counters();

        Vect v2 = std::move(v);
        assert(alloc_stats.count == 2);
        assert(alloc_stats.copied == 0);
        assert(alloc_stats.moved == 1);
        {
            const test_allocator<int>& a = v.get_allocator();
            assert(a.get_id() == test_alloc_base::moved_value);
            assert(a.get_data() == test_alloc_base::moved_value);
        }
        {
            const test_allocator<int>& a = v2.get_allocator();
            assert(a.get_id() == 101);
            assert(a.get_data() == 42);
        }
    }
}

namespace
{
    struct Nasty {
        TEST_CONSTEXPR Nasty() : i_(0)
        {}
        TEST_CONSTEXPR Nasty(int i) : i_(i)
        {}
        TEST_CONSTEXPR_CXX20 ~Nasty()
        {}

        Nasty* operator&() const
        {
            assert(false); return nullptr;
        }
        int i_;
    };
}

TEST(data)
{
    {
        std::vector<int> v;
        assert(v.data() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<Nasty> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v;
        assert(v.data() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<Nasty, min_allocator<Nasty>> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v;
        assert(v.data() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<Nasty, safe_allocator<Nasty>> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

TEST(clear)
{
    {
        int a[] = {1, 2, 3};
        std::vector<int> c(a, a + 3);
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.empty());
        LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
    {
        int a[] = {1, 2, 3};
        std::vector<int, min_allocator<int>> c(a, a + 3);
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.empty());
        LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    }
    {
        int a[] = {1, 2, 3};
        std::vector<int, safe_allocator<int>> c(a, a + 3);
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(c.empty());
        LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
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
        TEST_CONSTEXPR_CXX14 A(int i, double d)
            : i_(i), d_(d)
        {}

        TEST_CONSTEXPR_CXX14 A(A&& a)
            : i_(a.i_),
            d_(a.d_)
        {
            a.i_ = 0;
            a.d_ = 0;
        }

        TEST_CONSTEXPR_CXX14 A& operator=(A&& a)
        {
            i_ = a.i_;
            d_ = a.d_;
            a.i_ = 0;
            a.d_ = 0;
            return *this;
        }

        TEST_CONSTEXPR_CXX14 int geti() const
        {
            return i_;
        }
        TEST_CONSTEXPR_CXX14 double getd() const
        {
            return d_;
        }
    };
}

TEST(emplace)
{
    {
        std::vector<A> c;
        std::vector<A>::iterator i = c.emplace(c.cbegin(), 2, 3.5);
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        i = c.emplace(c.cend(), 3, 4.5);
        assert(i == c.end() - 1);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
        i = c.emplace(c.cbegin() + 1, 4, 6.5);
        assert(i == c.begin() + 1);
        assert(c.size() == 3);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c[1].geti() == 4);
        assert(c[1].getd() == 6.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, limited_allocator<A, 7> > c;
        std::vector<A, limited_allocator<A, 7> >::iterator i = c.emplace(c.cbegin(), 2, 3.5);
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        i = c.emplace(c.cend(), 3, 4.5);
        assert(i == c.end() - 1);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
        i = c.emplace(c.cbegin() + 1, 4, 6.5);
        assert(i == c.begin() + 1);
        assert(c.size() == 3);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c[1].geti() == 4);
        assert(c[1].getd() == 6.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, min_allocator<A> > c;
        std::vector<A, min_allocator<A> >::iterator i = c.emplace(c.cbegin(), 2, 3.5);
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        i = c.emplace(c.cend(), 3, 4.5);
        assert(i == c.end() - 1);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        i = c.emplace(c.cbegin() + 1, 4, 6.5);
        assert(i == c.begin() + 1);
        assert(c.size() == 3);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c[1].geti() == 4);
        assert(c[1].getd() == 6.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
    {
        std::vector<A, safe_allocator<A> > c;
        std::vector<A, safe_allocator<A> >::iterator i = c.emplace(c.cbegin(), 2, 3.5);
        assert(i == c.begin());
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        i = c.emplace(c.cend(), 3, 4.5);
        assert(i == c.end() - 1);
        assert(c.size() == 2);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        i = c.emplace(c.cbegin() + 1, 4, 6.5);
        assert(i == c.begin() + 1);
        assert(c.size() == 3);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c[1].geti() == 4);
        assert(c[1].getd() == 6.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
    }
}

TEST(emplace_back)
{
    {
        std::vector<A> c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, limited_allocator<A, 4> > c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, min_allocator<A> > c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<A, safe_allocator<A> > c;
    #if TEST_STD_VER > 14
        A& r1 = c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(&r1 == &c.back());
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        A& r2 = c.emplace_back(3, 4.5);
        assert(c.size() == 2);
        assert(&r2 == &c.back());
    #else
        c.emplace_back(2, 3.5);
        assert(c.size() == 1);
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(is_contiguous_container_asan_correct(c));
        c.emplace_back(3, 4.5);
        assert(c.size() == 2);
    #endif
        assert(c.front().geti() == 2);
        assert(c.front().getd() == 3.5);
        assert(c.back().geti() == 3);
        assert(c.back().getd() == 4.5);
        assert(is_contiguous_container_asan_correct(c));
    }
    {
        std::vector<Tag_X, TaggingAllocator<Tag_X> > c;
        c.emplace_back();
        assert(c.size() == 1);
        c.emplace_back(1, 2, 3);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
    }

    { // LWG 2164
        int arr[] = {0, 1, 2, 3, 4};
        int sz = 5;
        std::vector<int> c(arr, arr + sz);
        while (c.size() < c.capacity())
            c.push_back(sz++);
        c.emplace_back(c.front());
        assert(c.back() == 0);
        for (int i = 0; i < sz; ++i)
            assert(c[i] == i);
    }
}

TEST(emplace_extra)
{
    {
        std::vector<int> v;
        v.reserve(3);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v;
        v.reserve(4);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v;
        v.reserve(3);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v;
        v.reserve(4);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v;
        v.reserve(3);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, safe_allocator<int>> v;
        v.reserve(4);
        assert(is_contiguous_container_asan_correct(v));
        v = {1, 2, 3};
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v;
        v.reserve(8);
        std::size_t old_capacity = v.capacity();
        assert(old_capacity >= 8);

        v.resize(4); // keep the existing capacity
        assert(v.capacity() == old_capacity);

        v.emplace(v.cend(), 42);
        assert(v.size() == 5);
        assert(v.capacity() == old_capacity);
        assert(v[4] == 42);
    }
}

TEST(pop_back)
{
    {
        std::vector<int> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);

    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
    }
#endif

    { // LWG 526
        int arr[] = {0, 1, 2, 3, 4};
        int sz = 5;
        std::vector<int> c(arr, arr + sz);
        while (c.size() < c.capacity())
            c.push_back(sz++);
        c.push_back(c.front());
        assert(c.back() == 0);
        for (int i = 0; i < sz; ++i)
            assert(c[i] == i);
    }
}

TEST(push_back)
{
    {
        std::vector<int> c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
    {
        // libc++ needs 15 because it grows by 2x (1 + 2 + 4 + 8).
        // Use 17 for implementations that dynamically allocate a container proxy
        // and grow by 1.5x (1 for proxy + 1 + 2 + 3 + 4 + 6).
        std::vector<int, limited_allocator<int, 17> > c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
#endif
}

TEST(compare)
{
    {
        const std::vector<int> c1, c2;
        assert(testComparisons(c1, c2, true, false));
    }
    {
        const std::vector<int> c1(1, 1), c2(1, 2);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        const std::vector<int> c1, c2(1, 2);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        int items1[3] = {1, 2, 1};
        int items2[3] = {1, 2, 2};
        const std::vector<int> c1(items1, items1 + 3);
        const std::vector<int> c2(items2, items2 + 3);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        int items1[3] = {3, 2, 3};
        int items2[3] = {3, 1, 3};
        const std::vector<int> c1(items1, items1 + 3);
        const std::vector<int> c2(items2, items2 + 3);

        assert(testComparisons(c1, c2, false, false));
    }
    {
        int items1[2] = {1, 2};
        int items2[3] = {1, 2, 0};
        const std::vector<int> c1(items1, items1 + 2);
        const std::vector<int> c2(items2, items2 + 3);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        int items1[3] = {1, 2, 0};
        const std::vector<int> c1(items1, items1 + 3);
        const std::vector<int> c2(1, 3);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        const std::vector<LessAndEqComp> c1, c2;
        assert(testComparisons(c1, c2, true, false));
    }
    {
        const std::vector<LessAndEqComp> c1(1, LessAndEqComp(1));
        const std::vector<LessAndEqComp> c2(1, LessAndEqComp(1));
        assert(testComparisons(c1, c2, true, false));
    }
    {
        const std::vector<LessAndEqComp> c1(1, LessAndEqComp(1));
        const std::vector<LessAndEqComp> c2(1, LessAndEqComp(2));
        assert(testComparisons(c1, c2, false, true));
    }
    {
        const std::vector<LessAndEqComp> c1;
        const std::vector<LessAndEqComp> c2(1, LessAndEqComp(2));
        assert(testComparisons(c1, c2, false, true));
    }
    {
        LessAndEqComp items1[3] = {LessAndEqComp(1), LessAndEqComp(2), LessAndEqComp(2)};
        LessAndEqComp items2[3] = {LessAndEqComp(1), LessAndEqComp(2), LessAndEqComp(1)};
        const std::vector<LessAndEqComp> c1(items1, items1 + 3);
        const std::vector<LessAndEqComp> c2(items2, items2 + 3);
        assert(testComparisons(c1, c2, false, false));
    }
    {
        LessAndEqComp items1[3] = {LessAndEqComp(3), LessAndEqComp(3), LessAndEqComp(3)};
        LessAndEqComp items2[3] = {LessAndEqComp(3), LessAndEqComp(2), LessAndEqComp(3)};
        const std::vector<LessAndEqComp> c1(items1, items1 + 3);
        const std::vector<LessAndEqComp> c2(items2, items2 + 3);
        assert(testComparisons(c1, c2, false, false));
    }
    {
        LessAndEqComp items1[2] = {LessAndEqComp(1), LessAndEqComp(2)};
        LessAndEqComp items2[3] = {LessAndEqComp(1), LessAndEqComp(2), LessAndEqComp(0)};
        const std::vector<LessAndEqComp> c1(items1, items1 + 2);
        const std::vector<LessAndEqComp> c2(items2, items2 + 3);
        assert(testComparisons(c1, c2, false, true));
    }
    {
        LessAndEqComp items1[3] = {LessAndEqComp(1), LessAndEqComp(2), LessAndEqComp(0)};
        const std::vector<LessAndEqComp> c1(items1, items1 + 3);
        const std::vector<LessAndEqComp> c2(1, LessAndEqComp(3));
        assert(testComparisons(c1, c2, false, true));
    }
    {
        assert((std::vector<int>() == std::vector<int>()));
        assert(!(std::vector<int>() != std::vector<int>()));
        assert(!(std::vector<int>() < std::vector<int>()));
        assert((std::vector<int>() <= std::vector<int>()));
        assert(!(std::vector<int>() > std::vector<int>()));
        assert((std::vector<int>() >= std::vector<int>()));
    }
}

TEST()
{}

TEST()
{}

TEST()
{}

#endif
