#include "Test.h"


namespace UnitTest
{
    void TEST(Float2Int)()
    {
        // On x86, call _ftoui/_ftoui2/_ftoui3

        float Val1 = 1.6f;
        int   Val2 = static_cast<int>(Val1);

        ASSERT(Val2 == 1);
        MusaLOG("[*] 1.6f convert to int is %d", Val2);
    }
    TEST_PUSH(Float2Int);


}
