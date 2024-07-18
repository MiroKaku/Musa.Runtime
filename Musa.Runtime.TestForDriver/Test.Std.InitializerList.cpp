#include "Test.h"

#include <string>
#include <vector>
#include <unordered_map>


namespace UnitTest
{
    template<typename T>
    class TEST(InitializerListObject)
    {
        using iterator                  = typename std::vector<T>::iterator;
        using const_iterator            = typename std::vector<T>::const_iterator;
        using reverse_iterator          = typename std::vector<T>::reverse_iterator;
        using const_reverse_iterator    = typename std::vector<T>::const_reverse_iterator;

        std::vector<T> mVec;

    public:
        TEST(InitializerListObject)(std::initializer_list<T> List)
            : mVec(List)
        {}

        [[nodiscard]] const_iterator begin() const noexcept
        {
            return mVec.begin();
        }

        [[nodiscard]] iterator end() noexcept
        {
            return mVec.end();
        }

        [[nodiscard]] const_iterator cbegin() const noexcept
        {
            return mVec.cbegin();
        }

        [[nodiscard]] const_iterator cend() const noexcept
        {
            return mVec.cend();
        }
    };

    void TEST(InitializerList)()
    {
        auto Vec = TEST(InitializerListObject)<int>
        {
            0, 1, 2, 3, 4
        };

        int Idx = 0;
        for (const auto Val : Vec) {
            MusaLOG("[*] %d", Val);

            ASSERT(Idx == Val); ++Idx;
        }
    }
    TEST_PUSH(InitializerList);


    std::unordered_map<std::string, ULONG_PTR> TEST(StaticObjectInitializer) =
    {
        {"1", 1},
        {"2", 2},
        {"3", 3},
        {"4", 4},
        {"5", 5},
    };


}
