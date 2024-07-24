#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"


TEST()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

#endif
