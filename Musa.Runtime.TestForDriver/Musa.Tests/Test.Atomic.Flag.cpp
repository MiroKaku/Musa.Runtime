#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"


TEST(clear)
{
    {
        std::atomic_flag f;
        f.clear();
        f.test_and_set();
        std::atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        f.test_and_set();
        std::atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
}

TEST(clear_explicit)
{
    {
        std::atomic_flag f; // uninitialized first
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        std::atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
}

TEST(test)
{
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test(&f) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test(&f) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test(&f) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test(&f) == 1);
    }
}

TEST(test_and_set)
{
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set(&f) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set(&f) == 0);
        assert(f.test_and_set() == 1);
    }
}

TEST(test_and_set_explicit)
{
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
}

TEST(test_explicit)
{
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 1);
    }
#ifdef _LIBCPP_VERSION // Don't violate precondition [atomics.flag]/6
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_release) == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acq_rel) == 1);
    }
#endif // _LIBCPP_VERSION
    {
        std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_relaxed) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_consume) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acquire) == 1);
    }
#ifdef _LIBCPP_VERSION // Don't violate precondition [atomics.flag]/6
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_release) == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_acq_rel) == 1);
    }
#endif // _LIBCPP_VERSION
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 0);
        assert(std::atomic_flag_test_explicit(&f, std::memory_order_seq_cst) == 1);
    }
}

#endif
