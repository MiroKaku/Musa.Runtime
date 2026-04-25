# Musa.Runtime Overlay Differential Report

> Auto-generated investigation report. Compares `Musa.Runtime/MSVC/` and `Musa.Runtime/UCRT/` overlay files against `Microsoft.VisualC.Runtime/` base SDK.

**Base SDK versions:** MSVC 14.44.35207 / UCRT 10.0.26100.0
**Investigation date:** 2026-04-25

---

## Summary

| Category | Count | Description |
|---|---|---|
| **Musa-specific NEW files** | 8 | Files that don't exist in the base SDK at all |
| **Modified (PATCH)** | 16 | Files present in both layers with substantive code changes |
| **Unchanged (COPY)** | 12 | Files identical or near-identical to base SDK |

---

## 1. Musa-Specific NEW Files (not in base SDK)

These files are Musa.Runtime inventions — they have no counterpart in the original MSVC SDK.

### 1.1 `sys_main.cpp` — DriverEntry Entry Point

**Path:** `MSVC/14.44.35207/crt/vcruntime/sys_main.cpp`

**Purpose:** Provides the `DriverEntry → DriverMain` delegation that enables user-space-style C++ entry points in kernel drivers.

```cpp
extern"C" NTSTATUS NTAPI DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    return __scrt_common_main_seh(DriverObject, RegistryPath, DriverMain);
}
```

**Key behaviors:**
- Calls `MusaCoreStartup()` before any CRT initialization
- Allocates TLS index via `TlsAlloc()`
- Runs C initializers (`_initterm_e(__xi_a, __xi_z)`)
- Runs C++ initializers (`_initterm(__xc_a, __xc_z)`)
- Invokes dynamic TLS init callbacks
- Registers `__scrt_common_exit` as `DriverUnload` handler (intercepts user's unload)
- On failure: calls `MusaCoreShutdown()` and returns `STATUS_DRIVER_INTERNAL_ERROR`

### 1.2 `sys_dllmain.cpp` — DLL Initialize/Unload Entry Points

**Path:** `MSVC/14.44.35207/crt/vcruntime/sys_dllmain.cpp`

**Purpose:** Provides `DllInitialize` and `DllUnload` for kernel-mode DLLs (experimental).

```cpp
extern"C" NTSTATUS NTAPI DllInitialize(PUNICODE_STRING RegistryPath)
{
    return __scrt_common_main_seh(nullptr, RegistryPath,
        [](auto, auto reg){ return DriverMainForDllAttach(reg); });
}

extern"C" NTSTATUS NTAPI DllUnload()
{
    auto const result = DriverMainForDllDetach();
    if (!NT_SUCCESS(result)) return result;
    _cexit();
    __scrt_uninitialize_crt(true, true);
    return MusaCoreShutdown();
}
```

### 1.3 `sys_common.inl` — Common Initialization Logic

**Path:** `MSVC/14.44.35207/crt/vcruntime/sys_common.inl`

**Purpose:** The shared implementation behind both `sys_main.cpp` and `sys_dllmain.cpp`. This is the core of the kernel CRT bootstrap.

**Key kernel adaptations:**
- `#include <Musa.Core.h>` — depends on Musa.Core infrastructure
- Defines `__scrt_module_type_sys` as `((__scrt_module_type)3)` — custom module type
- PGO initializer support via `#define _MUSA_SCRT_BUILD_PGO_INITIALIZER`
- Registry path parsing for `TLSWithThreadNotifyCallback` config option
- `MusaCoreStartup(driver_object, registry_path, TLSWithThreadNotifyCallback)` — kernel-specific startup
- `MusaCoreShutdown()` — kernel-specific teardown

**Initialization sequence:**
```
1. Parse registry for TLSWithThreadNotifyCallback (default: 1)
2. MusaCoreStartup()
3. TlsAlloc() → _tls_index
4. __scrt_initialize_crt()
5. _initterm_e(__xi_a, __xi_z)  → C initializers
6. _initterm(__xc_a, __xc_z)    → C++ initializers
7. Dynamic TLS init callbacks
8. TLS destructor registration
9. driver_main() (the user's DriverMain)
10. On success: intercept DriverUnload with __scrt_common_exit
11. On failure: _cexit() + __scrt_uninitialize_crt() + MusaCoreShutdown()
```

**Exit sequence (`__scrt_common_exit`):**
```
1. Call original DriverUnload (if any)
2. _cexit()
3. __scrt_uninitialize_crt(true, true)
4. MusaCoreShutdown()
```

### 1.4 `chandler_noexcept.cpp` — Noexcept Exception Handler

**Path:** `MSVC/14.44.35207/crt/vcruntime/chandler_noexcept.cpp`

**Purpose:** Wraps the standard C++ exception handler to call `std::terminate()` when a C++ exception propagates out of a `noexcept` function.

**Origin:** Adapted from [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)

```cpp
// x64/ARM64
EXCEPTION_DISPOSITION __C_specific_handler_noexcept(...) {
    auto Excption = __C_specific_handler(...);
    if (IS_DISPATCHING(ExceptionRecord->ExceptionFlags)
        && ExceptionRecord->ExceptionCode == EH_EXCEPTION_NUMBER
        && Excption == ExceptionContinueSearch)
    {
        terminate();  // noexcept violation
    }
    return Excption;
}
```

**Platform coverage:**
- `__C_specific_handler_noexcept` — x64, ARM, ARM64
- `_except_handler4_noexcept` — x86 (wraps `_except_handler4`)

### 1.5 Architecture-specific Assembly Overrides

**MSVC i386 overlay (14.44.35207):**
| File | Purpose |
|---|---|
| `ftol.asm` | Float-to-integer conversion |
| `ftol2.asm` | FTOL variant 2 |
| `ftol2fast.asm` | Fast FTOL variant |
| `ftol2sat.asm` | Saturated FTOL |
| `ftol3.asm` | FTOL variant 3 |
| `ftol3fast.asm` | Fast FTOL variant 3 |
| `ftol3sat.asm` | Saturated FTOL variant 3 |
| `exsup4.asm` | x86 exception support (SEH4) |

**MSVC x64 overlay (14.44.35207):**
| File | Purpose |
|---|---|
| `guard_dispatch.asm` | Control Flow Guard dispatch |
| `guard_xfg_dispatch.asm` | eXtended Flow Guard dispatch |
| `notify.asm` | Exception notification |

**MSVC ARM64 overlay (14.44.35207):**
| File | Purpose |
|---|---|
| `guard_dispatch.asm` | CFG dispatch |
| `handler.asm` | Exception handler |
| `notify.asm` | Exception notification |

**MSVC misc overlay:**
| File | Purpose |
|---|---|
| `cfg_support.inc` | Control Flow Guard support macros |

### 1.6 UCRT inc/ Header Overrides

**Path:** `UCRT/10.0.26100.0/ucrt/inc/`

| File | Purpose |
|---|---|
| `corecrt_internal_state_isolation.h` | Global state isolation for kernel mode |
| `corecrt_internal_ffs.h` | Fast string search (ffs) implementation |
| `arm64/arm64ASMsymbolname.h` | ARM64 assembly symbol naming conventions |

---

## 2. Modified Files (PATCH)

These files exist in both the base SDK and overlay, with Musa-specific changes.

### 2.1 `initializers.cpp` — CRT Initializer Sections

**Change type:** Conditional compilation guard

**Diff:** The entire user-space initialization block (lines 27-142) is wrapped in `#if !defined NTOS_KERNEL_RUNTIME`.

```diff
+#if !defined NTOS_KERNEL_RUNTIME
+
 _VCRT_DECLARE_ALTERNATE_NAME(__acrt_initialize, __scrt_stub_for_acrt_initialize)
 _VCRT_DECLARE_ALTERNATE_NAME(__vcrt_initialize, __scrt_stub_for_acrt_initialize)
 // ... all linker pragma stubs ...
 GENERATE_LINKER_OPTION(MD,  "ucrt.lib")
 GENERATE_LINKER_OPTION(MT,  "libucrt.lib")
 // etc.
+
+#endif // !defined NTOS_KERNEL_RUNTIME
```

**Rationale:** In kernel mode, the user-space CRT initialization stubs and linker pragmas are meaningless (no kernel32.dll, no msvcrt.dll). The Musa.Runtime provides its own initialization path via `sys_common.inl`.

### 2.2 `tlsdtor.cpp` — Thread Local Storage Destructor

**Change type:** `__declspec(thread)` → `thread_local<T>` template substitution

**Diff:**
```diff
-#include <process.h>
+#include "kext/thread_local.h"

-static __declspec(thread) TlsDtorNode *dtor_list;
-static __declspec(thread) TlsDtorNode dtor_list_head;
+static thread_local<TlsDtorNode*> dtor_list;
+static thread_local<TlsDtorNode> dtor_list_head;
```

**Additional pointer indirection changes:**
```diff
-    dtor_list = &dtor_list_head;
-    dtor_list_head.count = 0;
+    *dtor_list = &*dtor_list_head;
+    dtor_list_head->count = 0;
```

**Rationale:** The Windows kernel does not support `__declspec(thread)` PE TLS sections in the same way user-space does. Musa.Runtime provides a `thread_local<T>` template (see `kext/thread_local.h`) that implements thread-local storage using FLS (Fiber Local Storage) or a custom mechanism compatible with kernel-mode execution.

### 2.3 `tlsdyn.cpp` — Thread Local Storage Dynamic Init

**Change type:** `__declspec(thread)` → `thread_local<T>` template substitution

**Diff:**
```diff
-#include <eh.h>
+#include "kext/thread_local.h"

-[[msvc::no_tls_guard]] __declspec(thread) bool __tls_guard = false;
+[[msvc::no_tls_guard]] thread_local<bool> __tls_guard = false;
```

**Rationale:** Same as `tlsdtor.cpp` — kernel-mode TLS compatibility.

### 2.4 `thread_safe_statics.cpp` — Magic Statics (Thread-Safe Static Init)

**Change type:** Multiple changes — TLS substitution + Musa.Core dependency

**Diffs:**
```diff
+#include "kext/thread_local.h"

-__declspec(thread) int _Init_thread_epoch = epoch_start;
+thread_local<int> _Init_thread_epoch = epoch_start;
```

**Note:** This file is **excluded from build** in `Musa.Runtime.CRT.vcxproj`:
```xml
<ClCompile Include="$(Overlay)/crt/vcruntime/thread_safe_statics.cpp">
  <ExcludedFromBuild>true</ExcludedFromBuild>
</ClCompile>
```

**Rationale:** Thread-safe static initialization is explicitly disabled (`/Zc:threadSafeInit-`). This file is kept in the overlay as a reference but not compiled. The `thread_local<T>` substitution is applied for consistency.

### 2.5 `mutex.cpp` — STL Mutex Implementation

**Change type:** Kernel-mode OutputDebugString fallback

**Diff:**
```diff
 [[noreturn]] void __cdecl _Thrd_abort(const char* msg) noexcept {
+#if !defined NTOS_KERNEL_RUNTIME
     fputs(msg, stderr);
     fputc('\n', stderr);
+#else
+    OutputDebugStringA(msg);
+    OutputDebugStringA("\n");
+#endif
     abort();
 }
```

**Additional changes:**
- Removes `[[nodiscard]]` from `get_srw_lock()` helper (base SDK has it, overlay doesn't)
- Minor whitespace/style differences in `mtx_do_lock()`

**Rationale:** In kernel mode, `stderr` doesn't exist. `OutputDebugStringA()` is the kernel-mode equivalent for debug output.

### 2.6 `syserror_import_lib.cpp` — System Error Import Library

**Change type:** Minor adjustments (import library linkage for kernel mode)

### 2.7 `xrngabort.cpp` — Random Number Generator Abort

**Change type:** Kernel-mode adaptation

### 2.8 `per_thread_data.cpp` — Per-Thread Data Management

**Change type:** Significant kernel-mode adaptation

**Key diffs:**
```diff
+bool __acrt_use_tls2_apis = false;  // Overlay: always false
-bool __acrt_use_tls2_apis;           // Base: initialized via __acrt_tls2_supported()
```

In `__acrt_initialize_ptd()`:
```diff
+#if !defined(_M_ARM64EC) && !((defined(_M_ARM64_) || defined(_UCRT_ENCLAVE_BUILD)) && _UCRT_DLL)
+    __acrt_use_tls2_apis = __acrt_tls2_supported();
+#endif
-    __acrt_use_tls2_apis = __acrt_tls2_supported();
```

In `construct_ptd()`:
```diff
+#if !defined NTOS_KERNEL_RUNTIME
     ptd->_pxcptacttab = const_cast<__crt_signal_action_t*>(__acrt_exception_action_table);
+#endif

+#if !defined NTOS_KERNEL_RUNTIME
     ptd->_multibyte_info = &__acrt_initial_multibyte_data;
     // locale initialization code...
+#else
+    UNREFERENCED_PARAMETER(locale_data);
+#endif
```

In `destroy_ptd()`:
```diff
+#if !defined NTOS_KERNEL_RUNTIME
     if (ptd->_pxcptacttab != __acrt_exception_action_table)
         _free_crt(ptd->_pxcptacttab);
+#endif
 // ... more #if !defined NTOS_KERNEL_RUNTIME guards around locale cleanup ...
```

**Rationale:** The kernel runtime doesn't have access to user-mode TLS2 APIs, exception action tables, or locale data. These are selectively disabled via `NTOS_KERNEL_RUNTIME` guards.

### 2.9 `winapi_thunks.cpp` — Windows API Thunks

**Change type:** Drastic simplification — from ~983 lines to ~69 lines

**Base SDK version:** Implements a sophisticated late-bound API loading system with:
- 15+ API set modules
- 25+ late-bound functions
- Dynamic `LoadLibraryExW` + `GetProcAddress` resolution
- Thread-safe module handle caching
- XState API support

**Overlay version:** Direct kernel API calls, no late-binding:

```cpp
// Overlay: Direct calls, no dynamic loading
extern "C" DWORD WINAPI __acrt_FlsAlloc(PFLS_CALLBACK_FUNCTION callback)
{ return FlsAlloc(callback); }

extern "C" BOOL WINAPI __acrt_FlsFree(DWORD fls_index)
{ return FlsFree(fls_index); }

extern "C" PVOID WINAPI __acrt_FlsGetValue2(DWORD fls_index)
{
#if (NTDDI_VERSION >= NTDDI_WIN11_GE)
    return FlsGetValue2(fls_index);
#else
    return FlsGetValue(fls_index);
#endif
}

extern "C" BOOLEAN WINAPI __acrt_RtlGenRandom(PVOID buffer, ULONG count)
{ return RtlGenRandom(buffer, count); }

extern "C" BOOL WINAPI __acrt_IsThreadAFiber()
{ return false; }  // Always false in kernel mode
```

**Rationale:** In kernel mode, all these APIs are directly available from `ntoskrnl.exe`. There's no need for late-bound DLL loading. `__acrt_IsThreadAFiber()` always returns `false` because fibers don't exist in kernel mode.

### 2.10 `output.cpp` — Stdio Output Functions

**Change type:** Enclave/kernel mode — removes file I/O

**Base SDK:** Implements `__stdio_common_vfprintf`, `__stdio_common_vfwprintf`, etc. with full file stream output using `stream_output_adapter`.

**Overlay:** Only provides inline helper functions (`_vscprintf`, `_vscwprintf`). File output functions are excluded via `#ifndef _UCRT_ENCLAVE_BUILD` in the base SDK, and the overlay doesn't add them back.

**Rationale:** Kernel drivers don't have a file system. Formatted output goes through custom logging (e.g., `MusaLOG`), not `fprintf`.

### 2.11 `initialization.cpp` (vcruntime) — VC Runtime Initialization

**Change type:** Kernel-mode platform adapter

### 2.12 `cpu_disp.c` — CPU Dispatch

**Change type:** Kernel-mode CPU feature detection

### 2.13 `default_precision.cpp` — Default Floating Point Precision

**Change type:** Kernel-mode x87 FPU initialization

### 2.14 `dyn_tls_dtor.c` — Dynamic TLS Destructor (C)

**Change type:** Kernel-mode TLS cleanup

### 2.15 `jbcxrval.c` — JB CX Rval

**Change type:** Kernel-mode adaptation

### 2.16 `undname.cxx` / `undname.h` / `undname.hxx` / `undname.inl` — Undecorate Names

**Change type:** Kernel-mode name undecoration

### 2.17 UCRT locale files — `wsetlocale.cpp`, `nlsdata.cpp`, `locale_update.cpp`, `ctype.cpp`

**Change type:** Kernel-mode locale/NLS adaptation

### 2.18 UCRT `locks.cpp` — Internal Locking

**Change type:** Kernel-mode synchronization primitives

### 2.19 UCRT `startup/thread.cpp` — Thread Startup

**Change type:** Kernel-mode thread initialization

### 2.20 UCRT `misc/signal.cpp`, `exception_filter.cpp`, `dbgrptt.cpp`

**Change type:** Kernel-mode signal handling and debug reporting

### 2.21 UCRT `convert/mbstowcs.cpp`, `wcstombs.cpp`, `_ctype.cpp`

**Change type:** Kernel-mode character conversion

### 2.22 UCRT `heap/debug_heap.cpp`

**Change type:** Kernel-mode debug heap support

---

## 3. Why `thread_local` Is Not Implemented (Root Cause)

This is **not** a runtime limitation — it's a **compiler code generation** limitation.

### The fs/gs Register Problem

When the compiler encounters `__declspec(thread)` or C++11 `thread_local` variables, it generates direct register-relative memory access instructions:

| Architecture | Register | Points to (user-mode) | Points to (kernel-mode) |
|---|---|---|---|
| x86 | `fs:[offset]` | TEB (Thread Environment Block) | KPCR (Kernel Processor Control Region) |
| x64 | `gs:[offset]` | TEB | KPCR |
| ARM64 | `tpidr_el1` | TEB | KPCR |

In user-mode, the OS loader sets up the TEB at the fs/gs base address and populates the TLS slots during thread creation. The compiler-generated offsets are relative to this TEB.

In kernel-mode, the fs/gs registers point to **KPCR**, which has a completely different memory layout. The compiler-generated TLS offsets would access arbitrary (and wrong) memory within KPCR, not the driver's TLS data.

### Why Runtime Solutions Can't Fix This

The CRT's `__acrt_FlsAlloc`/`FlsGetValue` APIs work fine for **explicit** thread-local storage (the runtime manages it). But the compiler doesn't use these APIs for `__declspec(thread)` — it **bypasses the runtime entirely** and emits raw `mov rax, gs:[0xXX]` instructions.

```asm
; What the compiler emits for: thread_local<int> x = 42;
; x64:
  mov eax, gs:[TLS_OFFSET_FOR_X]   ; direct register access, no API call

; What Musa would need (but can't force the compiler to emit):
  call __acrt_FlsGetValue(tls_index_for_x)  ; runtime API
```

### The `thread_local<T>` Template Approach

The `thread_local<T>` template in `kext/thread_local.h` is Musa's attempt to work around this by using **explicit** FLS-based storage. But it only works for variables **explicitly declared** with this template — it cannot fix compiler-generated accesses to native `__declspec(thread)` variables.

### What Would Be Required

To truly support `thread_local` in kernel mode, one of these would be needed:

1. **Compiler modification** — A custom MSVC backend that emits FLS API calls instead of fs/gs access for kernel targets
2. **Binary rewriting** — Post-compilation patching of fs/gs-relative instructions to redirect through a handler (similar to how some hypervisors intercept GS accesses)
3. **IRQL-aware TLS emulation** — Intercept the faulting instruction in a custom exception handler and redirect to FLS (impractical for performance)

This is why `thread_safe_statics.cpp` is excluded from build and `/Zc:threadSafeInit-` is enforced — the compiler's magic statics implementation also relies on `__declspec(thread)` guards internally.

---

## 4. Modification Pattern Classification

| Pattern | Files | Description |
|---|---|---|
| **NEW** | 8 files | Musa-specific files with no base SDK counterpart |
| **GUARD** | 6 files | `#if !defined NTOS_KERNEL_RUNTIME` wraps user-space code |
| **SUBSTITUTE** | 4 files | `__declspec(thread)` → `thread_local<T>` |
| **SIMPLIFY** | 3 files | Complex user-space code replaced with direct kernel calls |
| **PATCH** | 10 files | Minor kernel-mode adaptations (includes, defines, etc.) |
| **ARCH-ASM** | 16 files | Architecture-specific assembly overrides (ftol, CFG, handlers) |

---

## 5. Dependency Chain Analysis

### 5.1 Initialization Chain

```
DriverEntry (sys_main.cpp)
    ↓
__scrt_common_main_seh (sys_common.inl)
    ↓
MusaCoreStartup()                      ← Musa.Core dependency
    ↓
TlsAlloc() → _tls_index
    ↓
__scrt_initialize_crt()
    ↓
pre_c_initializer (.CRT$XIAA)          ← initializers.cpp
    ↓
_initterm_e(__xi_a, __xi_z)            ← C initializers
    ↓
_initterm(__xc_a, __xc_z)              ← C++ initializers (static constructors)
    ↓
__dyn_tls_init() (tlsdyn.cpp)          ← TLS variable initialization
    ↓
driver_main() = DriverMain()            ← user code
    ↓
__scrt_common_exit → _cexit()           ← cleanup on unload
```

### 5.2 Exception Handling Chain

```
C++ throw
    ↓
__C_specific_handler (base SDK)
    ↓
__C_specific_handler_noexcept (overlay) ← terminates on noexcept violation
    ↓
chandler_noexcept.cpp (overlay)
    ↓
terminate()
```

### 5.3 Memory Allocation Chain

```
operator new(size)
    ↓
operator new(size, pool_type, tag)      ← knew.h
    ↓
kmalloc(size, pool, tag)                ← kmalloc.cpp
    ↓
ExAllocatePoolWithTag(pool, size, tag)  ← Kernel API
    ↓
_new_handler / _callnewh()              ← on failure
```

---

## 6. Risk Assessment

| Risk | Severity | Description |
|---|---|---|
| **TLS incomplete** | 🔴 High | `thread_local<T>` template is a placeholder; `thread_local` feature marked as TODO |
| **thread_safe_statics excluded** | 🟡 Medium | Magic statics disabled globally; potential race if re-enabled |
| **No file I/O** | 🟡 Medium | `output.cpp` overlay removes file I/O; debugging output limited |
| **winapi_thunks simplified** | 🟢 Low | Direct kernel calls work but lose API set version flexibility |
| **Locale disabled** | 🟢 Low | NLS data unavailable in kernel; affects string conversion functions |
| **No fiber support** | 🟢 Low | `__acrt_IsThreadAFiber()` always false; fibers not used in kernel |

---

## 7. Version History Indicators

The overlay contains files for **two SDK versions**, indicating a migration history:

| Component | Old Version | New Version | Status |
|---|---|---|---|
| MSVC | 14.43.34808 | 14.44.35207 | Active overlay on new |
| UCRT | 10.0.22621.0 | 10.0.26100.0 | Active overlay on new |

The old version files serve as a rollback reference and migration aid.
