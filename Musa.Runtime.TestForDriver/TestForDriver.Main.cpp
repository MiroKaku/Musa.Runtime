#include <Veil.h>
#include "Test.h"

namespace Main
{
    EXTERN_C NTSTATUS DriverMain(const PDRIVER_OBJECT DriverObject, const PUNICODE_STRING Registry)
    {
        using namespace UnitTest;
        UNREFERENCED_PARAMETER(Registry);

        MusaLOG("Entry.");

        for (const auto& Test : TestVec) {
            Test();
        }

        DriverObject->DriverUnload = [](auto)
        {
            MusaLOG("Exit.");
        };

        return 0l;
    }

}
