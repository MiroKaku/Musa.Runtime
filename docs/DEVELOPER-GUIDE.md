# Musa.Runtime Developer Guide

## Quick Start

### Prerequisites

- Visual Studio 2022 (latest version)
- Windows Driver Kit (WDK)
- Git (with `--recurse-submodules` support)

### Installation

#### Method 1: NuGet Package (Recommended)

Right-click your project → **Manage NuGet Packages** → Search `Musa.Runtime` → Install.

Or add to your `.vcxproj`:

```xml
<ItemGroup>
  <PackageReference Include="Musa.Runtime">
    <Version>0.5.1</Version>
  </PackageReference>
</ItemGroup>
```

#### Method 2: Manual Import

Download from [Releases](https://github.com/MiroKaku/Musa.Runtime/releases), unzip, then add to your `.vcxproj`:

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
<!-- above Microsoft.Cpp.targets -->
<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
```

### First Step: Rename DriverEntry

**Musa.Runtime requires your driver entry point to be named `DriverMain`, not `DriverEntry`.**

The runtime provides its own `sys_main.cpp` which defines `DriverEntry` and delegates to `DriverMain`. If you keep your own `DriverEntry`, you will get a linker conflict.

```cpp
// Correct:
EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    // Your initialization code
    return STATUS_SUCCESS;
}
```

---

## Using C++ in Kernel Mode

### STL Containers

All standard STL containers are available. Use `kallocator<T>` for kernel-safe allocation:

```cpp
#include <unordered_map>
#include <string>
#include <vector>
#include "kext/kallocator.h"

void Example()
{
    // STL container with kernel pool allocator
    std::unordered_map<uint32_t, std::string, std::hash<uint32_t>,
        std::equal_to<uint32_t>, kallocator<std::pair<const uint32_t, std::string>>> map;

    map[1] = "hello";
    map[2] = "world";

    // Or use std::vector with kallocator
    std::vector<int, kallocator<int>> vec;
    vec.push_back(42);
}
```

**IRQL Warning:** The default `kallocator<T>` uses `PagedPool`. Do not use it at `DISPATCH_LEVEL` or above:

```cpp
// For DISPATCH_LEVEL+ usage, specify NonPagedPool explicitly:
std::vector<int, kallocator<int, NonPagedPool, 'Tag1>> safeVec;
```

### Exception Handling

Musa.Runtime supports C++ exceptions with `/EHa` (asynchronous handling):

```cpp
void TestException()
{
    try {
        try {
            throw std::wstring(L"kernel exception");
        }
        catch (int& e) {
            MusaLOG("Caught int: %d\n", e);
        }
    }
    catch (std::wstring& e) {
        MusaLOG("Caught wstring: %ls\n", e.c_str());
    }
    catch (...) {
        MusaLOG("Caught unknown exception\n");
    }
}
```

**Constraints:**
- Works at `IRQL <= APC_LEVEL` only
- Both `/EHa` and `/EHsc` are supported
- Stack unwinding through kernel frames is supported via SEH integration

### New/Delete

Standard `new` and `delete` operators work in kernel mode, backed by `ExAllocatePoolWithTag`:

```cpp
struct MyObject {
    int value;
    char buffer[256];
};

void TestNewDelete()
{
    auto* obj = new MyObject();
    obj->value = 42;
    delete obj;

    // Arrays
    auto* arr = new int[100]();
    delete[] arr;
}
```

### Static Objects

Global and static C++ objects are properly constructed at driver load time through the CRT initialization chain (`sys_main.cpp` → `initialization.cpp` → static constructors).

```cpp
// This static object will be constructed before DriverMain runs
static std::string g_config = "initialized";

EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING Registry)
{
    // g_config is already constructed here
    MusaLOG("Config: %s\n", g_config.c_str());
    return STATUS_SUCCESS;
}
```

---

## Kernel Memory API

### kmalloc / kfree

Low-level kernel memory allocation with pool type and tag control:

```cpp
#include "kext/kmalloc.h"

void Example()
{
    // Allocate from paged pool
    void* ptr = kmalloc(1024, PagedPool, 'Musa');
    if (ptr) {
        // Use ptr...
        kfree(ptr, 'Musa');
    }

    // Zero-initialized allocation
    void* zero = kcalloc(10, sizeof(int), NonPagedPool, 'Musa');

    // Resize
    void* resized = krealloc(ptr, 1024, 2048, PagedPool, 'Musa');
}
```

### kallocator Template

STL-compatible allocator for use with standard containers:

```cpp
#include "kext/kallocator.h"

// Default: PagedPool, tag='RsuM'
std::vector<int, kallocator<int>> vec1;

// Explicit pool and tag
std::vector<int, kallocator<int, NonPagedPool, 'Drv1'>> vec2;

// Using with maps (note the pair<const K, V> type)
using MapType = std::unordered_map<
    std::string,
    int,
    std::hash<std::string>,
    std::equal_to<std::string>,
    kallocator<std::pair<const std::string, int>>
>;
MapType map;
map["key"] = 42;
```

**Allocator tag convention:**
- `Musa.Veil` → Tag = `'MusV'`
- `Musa.Core` → Tag = `'MusC'`
- `Musa.Runtime` → Tag = `'RsuM'`

---

## Build Configuration

### Compiler Settings Applied by Musa.Runtime

| Setting | Value | Reason |
|---|---|---|
| Exception Handling | `/EHa` | Asynchronous — catches both C++ exceptions and SEH |
| Calling Convention | Cdecl | Standard C calling convention |
| Conformance Mode | `false` | SDK source requires non-conforming MSVC extensions |
| Thread-Safe Init | `/Zc:threadSafeInit-` | Disabled — kernel static initialization is single-threaded at load time |
| Debug Info | Program Database (Old Style) | Compatible with kernel debugging |
| Preprocessor | `_ONECORE`, `_KERNEL_MODE`, `NTOS_KERNEL_RUNTIME`, `_HAS_EXCEPTIONS` | Kernel mode + exception support |

### Header-Only Mode

If you only need the headers (e.g., building the runtime yourself in a separate project):

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>
</PropertyGroup>
```

This sets include paths but does **not** automatically link `Musa.Runtime.lib` or `Musa.Runtime.DriverEntry.obj`.

---

## Feature Status

| Feature | Status | Notes |
|---|---|---|
| New/Delete | ✅ Supported | Via ExAllocatePoolWithTag |
| C++ Exceptions | ✅ Supported | /EHa, IRQL ≤ APC_LEVEL |
| Static Objects | ✅ Supported | Constructed at driver load |
| SAFESEH / GS | ✅ Supported | Buffer security check enabled |
| STL (OneCore) | ✅ Supported | Full container/algorithm support |
| STL (CoreCRT) | ✅ Supported | Math/IO support |
| thread_local | ❌ Not yet | Planned for future release |
| /EHsc | ✅ Supported | Synchronous exception handling |
| ARM64 | ⚠️ Experimental | Builds but not fully tested |

---

## Testing

The `Musa.Runtime.TestForDriver` project demonstrates all runtime features:

```cpp
EXTERN_C NTSTATUS DriverMain(const PDRIVER_OBJECT DriverObject, const PUNICODE_STRING Registry)
{
    UNREFERENCED_PARAMETER(Registry);
    DriverObject->DriverUnload = [](auto obj) { MusaLOG("Exit."); };

    // Expand kernel stack for test execution
    return KeExpandKernelStackAndCalloutEx([](auto)
    {
        MusaLOG("Test started...");
        for (const auto& Test : TestVec) {
            Test();
        }
        MusaLOG("Test complete.");
    }, nullptr, MAXIMUM_EXPANSION_SIZE, TRUE, nullptr);
}
```

Tests run on an expanded kernel stack to avoid stack overflow during deep C++ call chains.

---

## Troubleshooting

### Linker Error: Unresolved External Symbol

Ensure `MusaRuntimeOnlyHeader` is set to `false` (or omitted) so that `Musa.Runtime.Config.targets` links the library:

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
```

### PAGE_FAULT_IN_NONPAGED_AREA BSOD

You are using `kallocator<T>` (default `PagedPool`) at `DISPATCH_LEVEL` or above. Specify `NonPagedPool`:

```cpp
std::vector<int, kallocator<int, NonPagedPool, 'Tag1>> vec;
```

### Static Variable Race Condition

Thread-safe static initialization is disabled (`/Zc:threadSafeInit-`). Do not rely on magic statics being thread-safe. Initialize all statics during `DriverMain` before any concurrent access.

### Version Mismatch Error

The build system validates that overlay and base UCRT versions match. If you see:

```
UCRT version mismatch: overlay is X but base is Y
```

Update both `Musa_Runtime_UCRT_Version_Overlay` and `Musa_Runtime_UCRT_Version` in `Musa.Runtime.props` to the same value.
