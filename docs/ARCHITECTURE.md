# Musa.Runtime Architecture

## Overview

Musa.Runtime is a Microsoft MSVC runtime library adapted for Windows kernel-mode development. It provides kernel developers with a C++ experience nearly identical to user-space application development — STL containers, exceptions, new/delete, static objects, and more — all running at kernel IRQL.

The project is built on [Musa.Core](https://github.com/MiroKaku/Musa.Core) and implements the architecture originally explored in [ucxxrt](https://github.com/MiroKaku/ucxxrt).

**Supported platforms:** x64 (stable), ARM64 (experimental), Win32 (limited)
**Target SDK versions:** MSVC 14.44.35207 / UCRT 10.0.26100.0

---

## Design Philosophy: Overlay Filesystem Pattern

Musa.Runtime adopts Docker's layered filesystem concept to achieve selective code overlay. Rather than forking and maintaining an independent copy of the MSVC runtime source, it layers Musa-specific modifications on top of the original SDK source tree.

```
┌─────────────────────────────────────────────────────┐
│                    Consumer Project                  │
│              (Kernel Driver / KMDF)                   │
├─────────────────────────────────────────────────────┤
│              Musa.Runtime.lib (aggregate)             │
│  ┌────────┬────────┬────────┬────────┬──────────┐   │
│  │  CRT   │  STL   │ VCRT   │  UCRT  │   Math   │   │
│  │ .lib   │ .lib   │ .lib   │ .lib   │   .lib   │   │
│  └───┬────┴───┬────┴───┬────┴───┬────┴────┬─────┘   │
│      │ overlay│ overlay│ overlay│ overlay│ extract   │
├──────┼────────┼────────┼────────┼────────┼───────────┤
│  ┌───┴────────┴──┐  ┌──┴────────┴───┐              │
│  │ Musa.Runtime/ │  │ Musa.Runtime/ │              │
│  │  MSVC/overlay │  │  UCRT/overlay │  ← TOP LAYER │
│  └───────┬───────┘  └───────┬───────┘              │
│          │                  │                       │
│  ┌───────┴──────────────────┴───────┐              │
│  │  Microsoft.VisualC.Runtime/      │              │
│  │    MSVC/14.44.35207/crt/         │              │
│  │    UCRT/10.0.26100.0/ucrt/       │  ← BASE LAYER│
│  └──────────────────────────────────┘              │
└─────────────────────────────────────────────────────┘
```

### Layer Mechanics

**Base Layer** (`Microsoft.VisualC.Runtime/`) — An unmodified copy of the original MSVC SDK source, organized by version:
- `MSVC/14.43.34808/` — Previous VC runtime version
- `MSVC/14.44.35207/` — Current VC runtime version
- `UCRT/10.0.22621.0/` — Previous UCRT version
- `UCRT/10.0.26100.0/` — Current UCRT version

**Top Layer** (`Musa.Runtime/MSVC/` and `Musa.Runtime/UCRT/`) — Musa-specific overrides that replace or extend base SDK files. The overlay mirrors the base directory structure (`crt/vcruntime/`, `ucrt/heap/`, etc.) so that path resolution naturally picks up the Musa version first.

### Include Path Resolution

The overlay is achieved through MSBuild include path ordering in `Musa.Runtime.props`:

```xml
<AdditionalIncludeDirectories>
  $(Musa_Runtime_VC_IncludePath_Overlay);    <!-- Musa overrides FIRST -->
  $(Musa_Runtime_UCRT_IncludePath_Overlay);
  $(Musa_Runtime_VC_IncludePath);            <!-- Base SDK SECOND -->
  $(Musa_Runtime_UCRT_IncludePath);
  %(AdditionalIncludeDirectories);
</AdditionalIncludeDirectories>
```

Files referenced by `$(Musa_Runtime_VC_ToolsInstallDir_Overlay)` come from the top layer; files referenced by `$(Musa_Runtime_VC_ToolsInstallDir)` come from the base layer. Each `.vcxproj` explicitly selects which layer each source file comes from.

### Version Consistency

The build system validates that overlay and base SDK versions match exactly:

```xml
<Target Name="ValidateMusaRuntimeVersions" BeforeTargets="PrepareForBuild">
  <Error Condition="'$(Musa_Runtime_UCRT_Version_Overlay)' != '$(Musa_Runtime_UCRT_Version)'"
         Text="UCRT version mismatch: overlay is $(Musa_Runtime_UCRT_Version_Overlay) but base is $(Musa_Runtime_UCRT_Version)." />
</Target>
```

This prevents ABI mismatches that would occur if overlay code were compiled against headers from a different SDK version than the base source.

---

## Component Architecture

Musa.Runtime is composed of five sub-libraries, each responsible for a distinct portion of the C/C++ runtime:

### 1. Musa.Runtime.CRT

**Role:** C runtime support — new/delete, SEH prolog/epilog, GS security checks, thread-local storage initialization, static object constructors.

**Key source files:**
- `kext/new_km.cpp`, `kext/delete_km.cpp` — Kernel-mode new/delete implementations using `ExAllocatePoolWithTag`/`ExFreePoolWithTag`
- `$(BaseSDK)/crt/vcruntime/` — Exception handling (ehvccctr, ehvecctr, ehvecdtr), delete operators, fltused, GS report, guard support, TLSSUP
- `$(Overlay)/crt/vcruntime/` — CPU dispatch, initializers, sys_main (DriverEntry rename), sys_dllmain, TLS dtor, utility
- Architecture-specific assembly: i386 (ftol variants), x64 (guard_dispatch, guard_xfg_dispatch), ARM64 (chkstk, secpushpop, guard_dispatch)

**Output:** `Musa.Runtime.CRT.lib`
**Special:** Publishes `sys_main.obj` as `Musa.Runtime.DriverEntry.obj` for the consumer's linker to pick up.

### 2. Musa.Runtime.STL

**Role:** Standard Template Library implementation — containers, algorithms, iostreams support, exception pointers, futures, threading primitives.

**Key source files:**
- `$(BaseSDK)/crt/stl/` — ~70 STL implementation files: atomic, charconv, exception pointers, futures, memory resource, mutex operations, math functions (xdint, xdnorm, etc.), thread support
- `$(Overlay)/crt/stl/` — mutex.cpp, syserror_import_lib.cpp, xrngabort.cpp (Musa-specific adaptations)
- Excluded from build: `special_math.cpp`, `xcosh.cpp`, `fcosh.cpp`, `fsinh.cpp`, `lcosh.cpp`, `lsinh.cpp`, `xsinh.cpp` (these are replaced by Boost.Math)

**Output:** `Musa.Runtime.STL.lib`

### 3. Musa.Runtime.VCRT

**Role:** Visual C++ Runtime — exception handling infrastructure, RTTI, frame management, type info, unexpected/uncaught exceptions.

**Key source files:**
- `$(BaseSDK)/crt/vcruntime/` — ehhelpers, ehstate, frame, locks, per_thread_data, purevirt, rtti, std_exception, std_type_info, throw, uncaught_exception(s), unexpected, user
- `$(Overlay)/crt/vcruntime/` — chandler_noexcept, initialization, jbcxrval.c, undname.cxx
- Architecture-specific: i386 (chandler4.c, trnsctrl.cpp, exsup4.asm), x64 (handlers.asm, notify.asm), ARM64 (handlers.asm, handler.asm, notify.asm, riscchandler.cpp, risctrnsctrl.cpp)

**Output:** `Musa.Runtime.VCRT.lib`

### 4. Musa.Runtime.UCRT

**Role:** Universal C Runtime — heap allocation, startup/exit, locale, stdlib, signal handling, internal initialization.

**Key source files:**
- `kext/kmalloc.cpp`, `kext/kfree.cpp` — Kernel memory allocation layer (`kmalloc`, `kfree`, `kcalloc`, `krealloc`, `krecalloc`) using Windows kernel pool APIs
- `$(BaseSDK)/ucrt/heap/` — malloc, calloc, free, realloc, recalloc, msize, new_handler, new_mode
- `$(BaseSDK)/ucrt/` — startup (abort, assert, exit, initterm, onexit), stdlib (abs, div, qsort, bsearch, rand, etc.), initializers
- `$(Overlay)/ucrt/` — convert (mbstowcs, wcstombs, _ctype), heap (debug_heap), internal (initialization, locks, OutputDebugStringA, per_thread_data, winapi_thunks), locale (locale_update, nlsdata, wsetlocale), misc (dbgrptt, exception_filter, signal), startup (thread), stdio (output)

**Output:** `Musa.Runtime.UCRT.lib`

### 5. Musa.Runtime.Math

**Role:** Math library extracted from the UCRT static library, with Musa-specific exclusions.

**Build process:** Unlike the other four libraries that compile individual source files, Math builds by extracting specific object files from the UCRT library using `lib.exe /REMOVE`:

```
Build.Microsoft.VisualC.Library.bat $(PlatformTarget) \
  -lib=libucrt[d].lib \
  -out=$(TargetPath) \
  -exclusion_list=Musa.Runtime.Math.Exclusion.txt
```

The exclusion file lists objects to remove from the extracted library. This approach ensures mathematical function consistency with the UCRT while allowing selective removal of kernel-incompatible code.

**Output:** `Musa.Runtime.Math.lib`

### Aggregate Library

**Musa.Runtime.vcxproj** links all five sub-libraries into a single `Musa.Runtime.lib`:

```xml
<Lib>
  <AdditionalDependencies>
    Musa.Runtime.CRT.lib;
    Musa.Runtime.STL.lib;
    Musa.Runtime.VCRT.lib;
    Musa.Runtime.UCRT.lib;
    Musa.Runtime.Math.lib;
  </AdditionalDependencies>
</Lib>
```

The aggregate library is what consumers link against.

---

## Kernel Mode Adaptations

### Memory Allocation

The standard heap allocator is replaced with Windows kernel pool allocation:

```
User-space:  malloc → HeapAlloc → Windows Heap
Kernel-mode: kmalloc → ExAllocatePoolWithTag → Kernel Pool
```

The `kallocator<T>` template provides an STL-compatible allocator that uses kernel pool:

```cpp
template <class _Ty, pool_t _Pool = PagedPool, unsigned long _Tag = 'RsuM'>
class kallocator {
    _Ty* allocate(size_t _Count) {
        return static_cast<_Ty*>(::operator new(
            _Get_size_of_n<sizeof(_Ty)>(_Count), _Pool, _Tag));
    }
    void deallocate(_Ty* _Ptr, size_t) {
        ::operator delete(_Ptr, _Pool, _Tag);
    }
};
```

**Warning:** Default pool type is `PagedPool`. Do not use `kallocator<T>` at `DISPATCH_LEVEL` or above without specifying `NonPagedPool` — accessing paged memory at elevated IRQL causes `PAGE_FAULT_IN_NONPAGED_AREA` BSOD.

### Exception Handling

- Mode: `/EHa` (asynchronous exception handling) — supports both C++ exceptions and SEH
- IRQL constraint: Exception handling works at `IRQL <= APC_LEVEL`
- Thread-safe statics: **Disabled** (`/Zc:threadSafeInit-`) — concurrent initialization of static local variables may race

### Key Preprocessor Definitions

| Symbol | Purpose |
|---|---|
| `_ONECORE` | OneCore target (reduced API surface) |
| `_KERNEL_MODE` / `NTOS_KERNEL_RUNTIME` | Kernel-mode compilation |
| `_HAS_EXCEPTIONS` | Enable C++ exceptions |
| `_CRTBLD` / `_VCRT_BUILD` / `_VCRT_ENCLAVE_BUILD` | Runtime build mode |
| `_STATIC_CPPLIB` | Static STL linkage |
| `DisableKernelFlag=true` | Override WDK's default C-only mode |

### Thread-Local Storage

- `thread_local` is **not yet supported** (listed as TODO in features)
- Static TLS initialization uses `dyn_tls_init.c`, `tlsdtor.cpp` from the overlay
- Dynamic TLS (`tlsdyn.cpp`) and thread-safe statics are excluded from build

---

## Build System

### Project Dependencies

```
Musa.Runtime.vcxproj
├── Musa.Runtime.CRT.vcxproj
├── Musa.Runtime.STL.vcxproj
├── Musa.Runtime.VCRT.vcxproj
├── Musa.Runtime.UCRT.vcxproj
└── Musa.Runtime.Math.vcxproj
```

### Build Entry Point

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> .\BuildAllTargets.cmd
```

`BuildAllTargets.cmd`:
1. Removes `Output/` for a clean build
2. Initializes Visual Studio environment (calls `InitializeVisualStudioEnvironment.cmd`)
3. Invokes MSBuild on `BuildAllTargets.proj` with binary logging

### Build Platform Support

- **x64** — Fully supported, primary target
- **ARM64** — Supported (experimental)
- **Win32** — Partially supported (platform props are commented out in most projects)

### SDK Version Configuration

Current versions defined in `Musa.Runtime.props`:
- `Musa_Runtime_VC_Version_Overlay` = `14.44.35207`
- `Musa_Runtime_UCRT_Version_Overlay` = `10.0.26100.0`

Previous versions also available in the base SDK for comparison or rollback.

---

## NuGet Package Structure

The NuGet package (`Musa.Runtime.nuspec`) distributes:
- `build/native/Musa.Runtime.props` — Build configuration for consumers
- `build/native/Musa.Runtime.Config.props` — Include/library path setup
- `build/native/Musa.Runtime.Config.targets` — Linker dependencies (`Musa.Runtime.lib`, `Musa.Runtime.DriverEntry.obj`)
- `build/native/Include/` — Public headers from `kext/`
- `build/native/Library/{Configuration}/{Platform}/` — Compiled `.lib` files

**Dependency:** `Musa.Core` (version >= 0.5.0)

### Header-Only Mode

Setting `<MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>` in a consumer's `.vcxproj` disables automatic lib import, useful for projects that only need the header definitions without linking the runtime.

---

## Reference Projects

- [Microsoft's C++ Standard Library (STL)](https://github.com/microsoft/stl) — Source of CRT/STL/VCRT code
- [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5) — Lightweight C runtime approach
- [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL) — Reference for overlay techniques
- [msvcr14x](https://github.com/sonyps5201314/msvcr14x) — MSVC runtime adaptation reference
- [Boost.Math](https://www.boost.org/doc/libs/release/libs/math/) — Submodule for special math functions
