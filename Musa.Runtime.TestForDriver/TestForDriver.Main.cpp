#include <Veil.h>
#include "Musa.Tests/Test.h"

namespace Main
{
    EXTERN_C VOID DriverExit(const PDRIVER_OBJECT DriverObject)
    {
        UNREFERENCED_PARAMETER(DriverObject);

        MusaLOG("Exit.");
    }

    EXTERN_C NTSTATUS DriverMain(const PDRIVER_OBJECT DriverObject, const PUNICODE_STRING Registry)
    {
        using namespace UnitTest;
        UNREFERENCED_PARAMETER(Registry);

        MusaLOG("Entry.");
        DriverObject->DriverUnload = DriverExit;

        for (const auto& Test : TestVec) {
            Test();
        }

        return 0l;
    }

}
