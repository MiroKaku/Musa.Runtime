#include "Test.h"


namespace UnitTest
{
    // compile with: /EHa
    void TEST(SEH)()
    {
        try {
            constexpr int InvalidAddress = 0;
            *reinterpret_cast<void**>(InvalidAddress) = TEST(SEH);
        }
        catch (...) {
            MusaLOG("[*] Caught a C exception");
        }
    }
    TEST_PUSH(SEH);


    // compile with: /EHa
    class TEST(StructExceptionTranslator)
    {
        unsigned int mCode = 0;
    public:
        TEST(StructExceptionTranslator) () = delete;

        TEST(StructExceptionTranslator)(const TEST(StructExceptionTranslator) & Other) noexcept
            : mCode(Other.mCode)
        {}

        TEST(StructExceptionTranslator)(TEST(StructExceptionTranslator) && Other) noexcept
            : mCode(Other.mCode)
        {
            Other.mCode = 0;
        }

        explicit TEST(StructExceptionTranslator)(const unsigned int Code) noexcept
            : mCode(Code)
        {}

        [[nodiscard]] unsigned int GetCode() const noexcept
        {
            return mCode;
        }
    };

    void TEST(SETranslate)()
    {
        _set_se_translator([](const unsigned int Code, _EXCEPTION_POINTERS* /*Exception*/)
        {
            MusaLOG("[*] In SE Translator.");
            throw TEST(StructExceptionTranslator)(Code);
        });

        try {
            constexpr int InvalidAddress = 0;
            *reinterpret_cast<void**>(InvalidAddress) = TEST(SETranslate);
        }
        catch (const TEST(StructExceptionTranslator)& Exception) {
            MusaLOG("[*] Caught a __try exception, error 0x%08X.", Exception.GetCode());
        }
    }
    TEST_PUSH(SETranslate);

}
