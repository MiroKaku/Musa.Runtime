#include "Test.h"


namespace UnitTest
{
    void TEST(ReAlloc)()
    {
        const auto Buffer1 = static_cast<int*>(malloc(sizeof(int)));

        if (Buffer1) {
            *Buffer1 = 123;

            const auto Buffer2 = static_cast<int*>(realloc(Buffer1, sizeof(size_t)));
            if (Buffer2) {
                MusaLOG("[*] The value of Buffer1 realloc to Buffer2 is %d", *Buffer2);

                ASSERT(*Buffer2 == 123);
                free(Buffer2);
            }
            else {
                free(Buffer1);
            }
        }
    }
    TEST_PUSH(ReAlloc);

}
