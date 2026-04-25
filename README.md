# [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) - Universal C++ RunTime Library

[![Actions Status](https://github.com/MiroKaku/Musa.Runtime/workflows/CI/badge.svg)](https://github.com/MiroKaku/Musa.Runtime/actions)
[![Downloads](https://img.shields.io/nuget/dt/Musa.Runtime?logo=NuGet&logoColor=blue)](https://www.nuget.org/packages/Musa.Runtime/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Runtime/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X64%7CARM64-%23FFBCD9)

* [English](README.md) · [简体中文](README.zh-CN.md)
* 📖 [Architecture](docs/ARCHITECTURE.md) · [Developer Guide](docs/DEVELOPER-GUIDE.md) · [Build System](docs/BUILD-SYSTEM.md) · [Overlay Diff Report](docs/OVERLAY-DIFF-REPORT.md)

---

## Overview

Musa.Runtime is a Microsoft MSVC runtime library adapted for Windows kernel-mode development. Built on [Musa.Core](https://github.com/MiroKaku/Musa.Core), it carries forward the architecture pioneered by [ucxxrt](https://github.com/MiroKaku/ucxxrt).

**Core goal:** Give kernel developers the same C++ experience that application developers enjoy.

**Design highlight:** Inspired by Docker's layered filesystem, Musa.Runtime uses an overlay mechanism to selectively replace files on top of the original MSVC SDK source — no fork, no independent copy to maintain. See [Architecture](docs/ARCHITECTURE.md) for details.

## Quick Start

### 1. Install

**NuGet (recommended):**

```xml
<ItemGroup>
  <PackageReference Include="Musa.Runtime">
    <Version>0.5.1</Version>
  </PackageReference>
</ItemGroup>
```

Or right-click your project → **Manage NuGet Packages** → search `Musa.Runtime` → Install.

**Manual import:** Download from [Releases](https://github.com/MiroKaku/Musa.Runtime/releases) and unzip:

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
<!-- place above Microsoft.Cpp.targets -->
```

### 2. Rename `DriverEntry` → `DriverMain`

Musa.Runtime provides its own `DriverEntry` (which handles CRT initialization). Your driver entry point must be renamed to `DriverMain`:

```cpp
EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DriverObject->DriverUnload = [](auto obj) { MusaLOG("Unload."); };
    // Your code here
    return STATUS_SUCCESS;
}
```

### 3. Use C++ in Kernel

```cpp
#include <vector>
#include <string>
#include "kext/kallocator.h"

void Example()
{
    std::vector<std::string, kallocator<std::string>> vec;
    vec.push_back("hello from kernel");
}
```

> ⚠️ `kallocator` defaults to `PagedPool`. When used at `DISPATCH_LEVEL` or above, specify `NonPagedPool` explicitly.

## Features

| Feature | Status | Notes |
|---|---|---|
| x64 | ✅ | Primary platform |
| ARM64 | ⚠️ | Experimental |
| New / Delete | ✅ | Backed by `ExAllocatePoolWithTag` |
| C++ Exception (`/EHa`, `/EHsc`) | ✅ | IRQL ≤ APC_LEVEL |
| Static Object Construction | ✅ | Runs at driver load time |
| SAFESEH / GS | ✅ | Buffer security check |
| STL (OneCore / CoreCRT) | ✅ | Full container / algorithm support |
| `thread_local` | ❌ | Compiler-level limitation (fs/gs register access) |

## Documentation

| Document | Content |
|---|---|
| [📐 Architecture](docs/ARCHITECTURE.md) | Design philosophy (overlay pattern), component architecture, kernel-mode adaptations |
| [📝 Developer Guide](docs/DEVELOPER-GUIDE.md) | Installation, STL usage, exception handling, kallocator, troubleshooting |
| [🔧 Build System](docs/BUILD-SYSTEM.md) | Project structure, build process, version management, publish pipeline |
| [🔬 Overlay Diff Report](docs/OVERLAY-DIFF-REPORT.md) | Per-file differential analysis of all overlay files |

## Build from Source

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> cd Musa.Runtime
> .\BuildAllTargets.cmd
```

Prerequisites: Visual Studio 2026 (latest) + WDK

## References

- [Microsoft's C++ Standard Library](https://github.com/microsoft/stl)
- [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)
- [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL)
- [msvcr14x](https://github.com/sonyps5201314/msvcr14x)
- [Boost.Math](https://www.boost.org/doc/libs/release/libs/math/)
