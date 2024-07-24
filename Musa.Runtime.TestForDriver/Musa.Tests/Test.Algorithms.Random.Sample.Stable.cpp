#ifdef TEST_HAS_ALG_RANDOM

#include <array>
#include <random>
#include <algorithm>

#include "LLVM.TestSuite/test_macros.h"
#include "LLVM.TestSuite/almost_satisfies_types.h"


namespace
{
    // Stable if and only if PopulationIterator meets the requirements of a
    // ForwardIterator type.
    template <class PopulationIterator, class SampleIterator>
    void test_stability(bool expect_stable)
    {
        const unsigned kPopulationSize = 100;
        int ia[kPopulationSize];
        for (unsigned i = 0; i < kPopulationSize; ++i)
            ia[i] = i;
        PopulationIterator first(ia);
        PopulationIterator last(ia + kPopulationSize);

        const unsigned kSampleSize = 20;
        int oa[kPopulationSize];
        SampleIterator out(oa);

        std::minstd_rand g;

        const int kIterations = 1000;
        bool unstable = false;
        for (int i = 0; i < kIterations; ++i) {
            std::sample(first, last, out, kSampleSize, g);
            unstable |= !std::is_sorted(oa, oa + kSampleSize);
        }
        assert(expect_stable == !unstable);
    }

}

TEST()
{
    test_stability<forward_iterator<int*>, cpp17_output_iterator<int*> >(true);
    test_stability<cpp17_input_iterator<int*>, random_access_iterator<int*> >(false);
}

#endif
