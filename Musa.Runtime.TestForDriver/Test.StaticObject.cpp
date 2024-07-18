#include "Test.h"


namespace UnitTest
{
    class TEST(StaticObject)
    {
        ULONG* mData = nullptr;

    public:

        TEST(StaticObject)() noexcept
            : mData(new ULONG[1]{1})
        {
            MusaLOG("[*] Has called.");
        }

        ~TEST(StaticObject)() noexcept
        {
            MusaLOG("[*] Has called.");

            ASSERT(mData[0] == 1);
            delete[] mData;
        }
    };

    _MAYBE_UNUSED static TEST(StaticObject) StaticObject;

}
