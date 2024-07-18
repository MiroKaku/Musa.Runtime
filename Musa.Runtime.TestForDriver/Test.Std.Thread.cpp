#include "Test.h"

#include <thread>
#include <mutex>
#include <condition_variable>


namespace UnitTest
{

    static std::mutex              TEST(Mutex);
    static std::condition_variable TEST(ConditionVariable);
    static std::string             TEST(ThreadData);
    static bool TEST(ThreadReady)     = false;
    static bool TEST(ThreadProcessed) = false;

    void TEST(Thread)()
    {
        auto Worker = std::thread([]
        {
            std::unique_lock<std::mutex> Lock(TEST(Mutex));
            TEST(ConditionVariable).wait(Lock, []
            {
                return TEST(ThreadReady);
            });

            MusaLOG("[*] Worker thread is processing data");
            TEST(ThreadData) += " after processing";

            TEST(ThreadProcessed) = true;
            MusaLOG("[*] Worker thread signals data processing completed");

            Lock.unlock();
            TEST(ConditionVariable).notify_one();
        });

        TEST(ThreadData) = "Example data";
        {
            std::unique_lock<std::mutex> Lock(TEST(Mutex));
            TEST(ThreadReady) = true;
            MusaLOG("[*] Main() signals data ready for processing");
        }
        TEST(ConditionVariable).notify_one();

        {
            std::unique_lock<std::mutex> Lock(TEST(Mutex));
            TEST(ConditionVariable).wait(Lock, []
            {
                return TEST(ThreadProcessed);
            });
        }
        MusaLOG("[*] Back in main(), data = %s", TEST(ThreadData).c_str());

        Worker.join();
    }
    TEST_PUSH(Thread);

}
