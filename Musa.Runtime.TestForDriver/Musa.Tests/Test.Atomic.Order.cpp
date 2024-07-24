#ifdef TEST_HAS_ATOMIC

#include <atomic>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/atomic_helpers.h"


TEST(kill_dependency)
{
    assert(std::kill_dependency(5) == 5);
    assert(std::kill_dependency(-5.5) == -5.5);
}

#endif
