#include <Veil.h>
#include "Musa.Tests/Test.h"


namespace Main
{
    using namespace UnitTest;

    EXTERN_C VOID DriverExit(const PDRIVER_OBJECT DriverObject)
    {
        UNREFERENCED_PARAMETER(DriverObject);

        MusaLOG("Exit.");
    }

    EXTERN_C NTSTATUS DriverMain(const PDRIVER_OBJECT DriverObject, const PUNICODE_STRING Registry)
    {
        UNREFERENCED_PARAMETER(Registry);

        MusaLOG("Entry.");
        DriverObject->DriverUnload = DriverExit;

        return KeExpandKernelStackAndCalloutEx([](auto)
        {
            MusaLOG("Start testing ...");
            for (const auto& Test : TestVec) {
                Test();
            }
            MusaLOG("Complete test.");

        }, nullptr, MAXIMUM_EXPANSION_SIZE, TRUE, nullptr);
    }

}
