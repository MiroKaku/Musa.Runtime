#include "Test.h"

#include <string>
#include <random>
#include <unordered_map>


namespace UnitTest
{
    void TEST(UnorderedMap)()
    {
        auto Sand = LARGE_INTEGER();
        KeQueryTickCount(&Sand);

        auto Rand = std::mt19937_64(Sand.QuadPart);
        auto Map  = std::unordered_map<uint32_t, std::string>();

        for (auto Idx = 0u; Idx < 10; ++Idx) {
            Map[Idx] = std::to_string(Rand());
        }

    #if _MSVC_LANG < 201703L
        for (const auto& Item : Map) {
            MusaLOG("[*] Map[%ld] = %s", Item.first, Item.second.c_str());
        }
    #else
        for (const auto& [Idx, Val] : Map) {
            MusaLOG("[*] Map[%ld] = %s", Idx, Val.c_str());
        }
    #endif
    }
    TEST_PUSH(UnorderedMap);


}
