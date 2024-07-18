#include "Test.h"

#include <string>


namespace UnitTest
{
    void TEST(ThrowInt)()
    {
        try {
            try {
                try {
                    throw 1;
                }
                catch (const int& Exception) {
                    MusaLOG("[*] Catch exception: %d", Exception);
                }
            }
            catch (const std::string& Exception) {
                ASSERT(false);
                MusaLOG("[x] Catch exception: %s", Exception.c_str());
            }
        }
        catch (...) {
            ASSERT(false);
            MusaLOG("[x] Catch exception: ...");
        }
    }
    TEST_PUSH(ThrowInt);


    void TEST(ThrowObject)()
    {
        try {
            try {
                try {
                    throw std::string(__FUNCTION__);
                }
                catch (const int& Exception) {
                    ASSERT(false);
                    MusaLOG("[x] Catch exception: %d", Exception);
                }
            }
            catch (const std::string& Exception) {
                MusaLOG("[*] Catch exception: %s", Exception.c_str());
            }
        }
        catch (...) {
            ASSERT(false);
            MusaLOG("[x] Catch exception: ...");
        }
    }
    TEST_PUSH(ThrowObject);


    void TEST(ThrowUnknow)()
    {
        try {
            try {
                try {
                    throw std::wstring(__FUNCTIONW__);
                }
                catch (const int& Exception) {
                    ASSERT(false);
                    MusaLOG("[x] Catch exception: %d", Exception);
                }
            }
            catch (const std::string& Exception) {
                ASSERT(false);
                MusaLOG("[x] Catch exception: %s", Exception.c_str());
            }
        }
        catch (...) {
            MusaLOG("[*] Catch exception: ...");
        }
    }
    TEST_PUSH(ThrowUnknow);

}
