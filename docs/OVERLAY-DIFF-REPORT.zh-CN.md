# Musa.Runtime 覆盖层差异报告

> 自动生成的调查报告。比较 `Musa.Runtime/MSVC/` 和 `Musa.Runtime/UCRT/` 覆盖层文件与 `Microsoft.VisualC.Runtime/` 基础 SDK。

**基础 SDK 版本：** MSVC 14.44.35207 / UCRT 10.0.26100.0
**调查日期：** 2026-04-25

---

## 摘要

| 类别 | 数量 | 描述 |
|---|---|---|
| **Musa 新增文件** | 8 | 基础 SDK 中完全不存在的文件 |
| **已修改 (PATCH)** | 16 | 两个层都存在且有实质性代码变更的文件 |
| **未变更 (COPY)** | 12 | 与基础 SDK 相同或几乎相同的文件 |

---

## 1. Musa 专用新增文件（基础 SDK 中不存在）

这些文件是 Musa.Runtime 的创新——它们在原始 MSVC SDK 中没有对应的文件。

### 1.1 `sys_main.cpp` — DriverEntry 入口点

**路径：** `MSVC/14.44.35207/crt/vcruntime/sys_main.cpp`

**用途：** 提供 `DriverEntry → DriverMain` 委托，使内核驱动程序能够使用用户空间风格的 C++ 入口点。

```cpp
extern"C" NTSTATUS NTAPI DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    return __scrt_common_main_seh(DriverObject, RegistryPath, DriverMain);
}
```

**关键行为：**
- 在任何 CRT 初始化之前调用 `MusaCoreStartup()`
- 通过 `TlsAlloc()` 分配 TLS 索引
- 运行 C 初始化器 (`_initterm_e(__xi_a, __xi_z)`)
- 运行 C++ 初始化器 (`_initterm(__xc_a, __xc_z)`)
- 调用动态 TLS 初始化回调
- 将 `__scrt_common_exit` 注册为 `DriverUnload` 处理程序（拦截用户的卸载）
- 失败时：调用 `MusaCoreShutdown()` 并返回 `STATUS_DRIVER_INTERNAL_ERROR`

### 1.2 `sys_dllmain.cpp` — DLL 初始化/卸载入口点

**路径：** `MSVC/14.44.35207/crt/vcruntime/sys_dllmain.cpp`

**用途：** 为内核模式 DLL（实验性）提供 `DllInitialize` 和 `DllUnload`。

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

### 1.3 `sys_common.inl` — 公共初始化逻辑

**路径：** `MSVC/14.44.35207/crt/vcruntime/sys_common.inl`

**用途：** `sys_main.cpp` 和 `sys_dllmain.cpp` 背后的共享实现。这是内核 CRT 启动的核心。

**关键内核适配：**
- `#include <Musa.Core.h>` — 依赖 Musa.Core 基础设施
- 定义 `__scrt_module_type_sys` 为 `((__scrt_module_type)3)` — 自定义模块类型
- 通过 `#define _MUSA_SCRT_BUILD_PGO_INITIALIZER` 支持 PGO 初始化器
- 解析注册表以获取 `TLSWithThreadNotifyCallback` 配置选项
- `MusaCoreStartup(driver_object, registry_path, TLSWithThreadNotifyCallback)` — 内核特定启动
- `MusaCoreShutdown()` — 内核特定清理

**初始化序列：**
```
1. 解析注册表获取 TLSWithThreadNotifyCallback（默认值：1）
2. MusaCoreStartup()
3. TlsAlloc() → _tls_index
4. __scrt_initialize_crt()
5. _initterm_e(__xi_a, __xi_z)  → C 初始化器
6. _initterm(__xc_a, __xc_z)    → C++ 初始化器
7. 动态 TLS 初始化回调
8. TLS 析构函数注册
9. driver_main()（用户的 DriverMain）
10. 成功时：用 __scrt_common_exit 拦截 DriverUnload
11. 失败时：_cexit() + __scrt_uninitialize_crt() + MusaCoreShutdown()
```

**退出序列（`__scrt_common_exit`）：**
```
1. 调用原始 DriverUnload（如果有）
2. _cexit()
3. __scrt_uninitialize_crt(true, true)
4. MusaCoreShutdown()
```

### 1.4 `chandler_noexcept.cpp` — Noexcept 异常处理程序

**路径：** `MSVC/14.44.35207/crt/vcruntime/chandler_noexcept.cpp`

**用途：** 包装标准 C++ 异常处理程序，当 C++ 异常从 `noexcept` 函数中传播时调用 `std::terminate()`。

**来源：** 改编自 [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)

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

**平台覆盖：**
- `__C_specific_handler_noexcept` — x64, ARM, ARM64
- `_except_handler4_noexcept` — x86（包装 `_except_handler4`）

### 1.5 架构特定汇编覆盖

**MSVC i386 覆盖层 (14.44.35207)：**

| 文件 | 用途 |
|---|---|
| `ftol.asm` | 浮点数到整数转换 |
| `ftol2.asm` | FTOL 变体 2 |
| `ftol2fast.asm` | 快速 FTOL 变体 |
| `ftol2sat.asm` | 饱和 FTOL |
| `ftol3.asm` | FTOL 变体 3 |
| `ftol3fast.asm` | 快速 FTOL 变体 3 |
| `ftol3sat.asm` | 饱和 FTOL 变体 3 |
| `exsup4.asm` | x86 异常支持 (SEH4) |

**MSVC x64 覆盖层 (14.44.35207)：**

| 文件 | 用途 |
|---|---|
| `guard_dispatch.asm` | 控制流Guard调度 |
| `guard_xfg_dispatch.asm` | 扩展流Guard调度 |
| `notify.asm` | 异常通知 |

**MSVC ARM64 覆盖层 (14.44.35207)：**

| 文件 | 用途 |
|---|---|
| `guard_dispatch.asm` | CFG 调度 |
| `handler.asm` | 异常处理程序 |
| `notify.asm` | 异常通知 |

**MSVC 杂项覆盖层：**

| 文件 | 用途 |
|---|---|
| `cfg_support.inc` | 控制流Guard支持宏 |

### 1.6 UCRT inc/ 头文件覆盖

**路径：** `UCRT/10.0.26100.0/ucrt/inc/`

| 文件 | 用途 |
|---|---|
| `corecrt_internal_state_isolation.h` | 内核模式全局状态隔离 |
| `corecrt_internal_ffs.h` | 快速字符串搜索 (ffs) 实现 |
| `arm64/arm64ASMsymbolname.h` | ARM64 汇编符号命名约定 |

---

## 2. 已修改文件 (PATCH)

这些文件在基础 SDK 和覆盖层中都存在，但有 Musa 特定的更改。

### 2.1 `initializers.cpp` — CRT 初始化节

**更改类型：** 条件编译保护

**差异：** 整个用户空间初始化块（第 27-142 行）被包装在 `#if !defined NTOS_KERNEL_RUNTIME` 中。

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

**理由：** 在内核模式中，用户空间 CRT 初始化存根和链接器 pragma 没有意义（没有 kernel32.dll，没有 msvcrt.dll）。Musa.Runtime 通过 `sys_common.inl` 提供自己的初始化路径。

### 2.2 `tlsdtor.cpp` — 线程本地存储析构函数

**更改类型：** `__declspec(thread)` → `thread_local<T>` 模板替换

**差异：**
```diff
-#include <process.h>
+#include "kext/thread_local.h"

-static __declspec(thread) TlsDtorNode *dtor_list;
-static __declspec(thread) TlsDtorNode dtor_list_head;
+static thread_local<TlsDtorNode*> dtor_list;
+static thread_local<TlsDtorNode> dtor_list_head;
```

**额外的指针间接更改：**
```diff
-    dtor_list = &dtor_list_head;
-    dtor_list_head.count = 0;
+    *dtor_list = &*dtor_list_head;
+    dtor_list_head->count = 0;
```

**理由：** Windows 内核不支持用户空间那样的 `__declspec(thread)` PE TLS 节。Musa.Runtime 提供了一个 `thread_local<T>` 模板（参见 `kext/thread_local.h`），使用 FLS（纤程本地存储）或与内核模式兼容的自定义机制实现线程本地存储。

### 2.3 `tlsdyn.cpp` — 线程本地存储动态初始化

**更改类型：** `__declspec(thread)` → `thread_local<T>` 模板替换

**差异：**
```diff
-#include <eh.h>
+#include "kext/thread_local.h"

-[[msvc::no_tls_guard]] __declspec(thread) bool __tls_guard = false;
+[[msvc::no_tls_guard]] thread_local<bool> __tls_guard = false;
```

**理由：** 与 `tlsdtor.cpp` 相同——内核模式 TLS 兼容性。

### 2.4 `thread_safe_statics.cpp` — 魔法静态量（线程安全静态初始化）

**更改类型：** 多个更改——TLS 替换 + Musa.Core 依赖

**差异：**
```diff
+#include "kext/thread_local.h"

-__declspec(thread) int _Init_thread_epoch = epoch_start;
+thread_local<int> _Init_thread_epoch = epoch_start;
```

**注意：** 此文件在 `Musa.Runtime.CRT.vcxproj` 中**被排除构建**：
```xml
<ClCompile Include="$(Overlay)/crt/vcruntime/thread_safe_statics.cpp">
  <ExcludedFromBuild>true</ExcludedFromBuild>
</ClCompile>
```

**理由：** 线程安全静态初始化被明确禁用（`/Zc:threadSafeInit-`）。此文件保留在覆盖层中作为参考但不编译。应用 `thread_local<T>` 替换以保持一致性。

### 2.5 `mutex.cpp` — STL 互斥锁实现

**更改类型：** 内核模式 OutputDebugString 回退

**差异：**
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

**其他更改：**
- 从 `get_srw_lock()` 辅助函数中移除 `[[nodiscard]]`（基础 SDK 有，覆盖层没有）
- `mtx_do_lock()` 中的微小空格/样式差异

**理由：** 在内核模式中，`stderr` 不存在。`OutputDebugStringA()` 是内核模式的等效调试输出。

### 2.6 `syserror_import_lib.cpp` — 系统错误导入库

**更改类型：** 微小调整（内核模式导入库链接）

### 2.7 `xrngabort.cpp` — 随机数生成器中止

**更改类型：** 内核模式适配

### 2.8 `per_thread_data.cpp` — 每线程数据管理

**更改类型：** 重大内核模式适配

**关键差异：**
```diff
+bool __acrt_use_tls2_apis = false;  // Overlay: always false
-bool __acrt_use_tls2_apis;           // Base: initialized via __acrt_tls2_supported()
```

在 `__acrt_initialize_ptd()` 中：
```diff
+#if !defined(_M_ARM64EC) && !((defined(_M_ARM64_) || defined(_UCRT_ENCLAVE_BUILD)) && _UCRT_DLL)
+    __acrt_use_tls2_apis = __acrt_tls2_supported();
+#endif
-    __acrt_use_tls2_apis = __acrt_tls2_supported();
```

在 `construct_ptd()` 中：
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

在 `destroy_ptd()` 中：
```diff
+#if !defined NTOS_KERNEL_RUNTIME
     if (ptd->_pxcptacttab != __acrt_exception_action_table)
         _free_crt(ptd->_pxcptacttab);
+#endif
 // ... more #if !defined NTOS_KERNEL_RUNTIME guards around locale cleanup ...
```

**理由：** 内核运行时无法访问用户模式 TLS2 API、异常操作表或区域设置数据。这些通过 `NTOS_KERNEL_RUNTIME` 保护有选择地禁用。

### 2.9 `winapi_thunks.cpp` — Windows API 存根

**更改类型：** 大幅简化——从约 983 行简化到约 69 行

**基础 SDK 版本：** 实现了一个复杂的延迟绑定 API 加载系统，包括：
- 15+ API 集模块
- 25+ 延迟绑定函数
- 动态 `LoadLibraryExW` + `GetProcAddress` 解析
- 线程安全模块句柄缓存
- XState API 支持

**覆盖层版本：** 直接内核 API 调用，无延迟绑定：

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

**理由：** 在内核模式中，所有这些 API 都可直接从 `ntoskrnl.exe` 获取。不需要延迟绑定的 DLL 加载。`__acrt_IsThreadAFiber()` 始终返回 `false`，因为内核模式中不存在纤程。

### 2.10 `output.cpp` — 标准 I/O 输出函数

**更改类型：** Enclave/内核模式——移除文件 I/O

**基础 SDK：** 使用 `stream_output_adapter` 实现 `__stdio_common_vfprintf`、`__stdio_common_vfwprintf` 等，完整文件流输出。

**覆盖层：** 仅提供内联辅助函数（`_vscprintf`、`_vscwprintf`）。文件输出函数在基础 SDK 中通过 `#ifndef _UCRT_ENCLAVE_BUILD` 排除，覆盖层没有添加回来。

**理由：** 内核驱动程序没有文件系统。格式化输出通过自定义日志（例如 `MusaLOG`）传递，而不是 `fprintf`。

### 2.11 `initialization.cpp` (vcruntime) — VC 运行时初始化

**更改类型：** 内核模式平台适配器

### 2.12 `cpu_disp.c` — CPU 分发

**更改类型：** 内核模式 CPU 特性检测

### 2.13 `default_precision.cpp` — 默认浮点精度

**更改类型：** 内核模式 x87 FPU 初始化

### 2.14 `dyn_tls_dtor.c` — 动态 TLS 析构函数 (C)

**更改类型：** 内核模式 TLS 清理

### 2.15 `jbcxrval.c` — JB CX Rval

**更改类型：** 内核模式适配

### 2.16 `undname.cxx` / `undname.h` / `undname.hxx` / `undname.inl` — 名称去装饰

**更改类型：** 内核模式名称去装饰

### 2.17 UCRT 区域设置文件 — `wsetlocale.cpp`, `nlsdata.cpp`, `locale_update.cpp`, `ctype.cpp`

**更改类型：** 内核模式区域设置/NLS 适配

### 2.18 UCRT `locks.cpp` — 内部锁

**更改类型：** 内核模式同步原语

### 2.19 UCRT `startup/thread.cpp` — 线程启动

**更改类型：** 内核模式线程初始化

### 2.20 UCRT `misc/signal.cpp`, `exception_filter.cpp`, `dbgrptt.cpp`

**更改类型：** 内核模式信号处理和调试报告

### 2.21 UCRT `convert/mbstowcs.cpp`, `wcstombs.cpp`, `_ctype.cpp`

**更改类型：** 内核模式字符转换

### 2.22 UCRT `heap/debug_heap.cpp`

**更改类型：** 内核模式调试堆支持

---

## 3. 为什么 `thread_local` 未实现（根本原因）

这不是**运行时限制**，而是**编译器代码生成限制**。

### fs/gs 寄存器问题

当编译器遇到 `__declspec(thread)` 或 C++11 `thread_local` 变量时，它生成直接寄存器相关的内存访问指令：

| 架构 | 寄存器 | 指向（用户模式） | 指向（内核模式） |
|---|---|---|---|
| x86 | `fs:[offset]` | TEB（线程环境块） | KPCR（内核处理器控制区） |
| x64 | `gs:[offset]` | TEB | KPCR |
| ARM64 | `tpidr_el1` | TEB | KPCR |

在用户模式中，操作系统加载器在 fs/gs 基地址设置 TEB，并在线程创建时填充 TLS 槽。编译器生成的偏移量相对于这个 TEB。

在内核模式中，fs/gs 寄存器指向 **KPCR**，它具有完全不同的内存布局。编译器生成的 TLS 偏移量将访问 KPCR 中的任意（和错误的）内存，而不是驱动程序的 TLS 数据。

### 为什么运行时解决方案无法修复此问题

CRT 的 `__acrt_FlsAlloc`/`FlsGetValue` API 对**显式**线程本地存储（运行时管理）正常工作。但编译器不对 `__declspec(thread)` 使用这些 API——它**完全绕过运行时**，发出原始的 `mov rax, gs:[0xXX]` 指令。

```asm
; What the compiler emits for: thread_local<int> x = 42;
; x64:
  mov eax, gs:[TLS_OFFSET_FOR_X]   ; direct register access, no API call

; What Musa would need (but can't force the compiler to emit):
  call __acrt_FlsGetValue(tls_index_for_x)  ; runtime API
```

### `thread_local<T>` 模板方法

`kext/thread_local.h` 中的 `thread_local<T>` 模板是 Musa 尝试使用**显式**基于 FLS 的存储来解决这个问题的方法。但它仅对**使用此模板显式声明**的变量有效——无法修复编译器对原生 `__declspec(thread)` 变量生成的访问。

### 需要什么

要在内核模式中真正支持 `thread_local`，需要以下之一：

1. **编译器修改** — 针对内核目标，发出 FLS API 调用而不是 fs/gs 访问的自定义 MSVC 后端
2. **二进制重写** — 编译后修补 fs/gs 相关指令以重定向到处理程序（类似于某些虚拟机管理程序拦截 GS 访问的方式）
3. **IRQL 感知 TLS 仿真** — 在自定义异常处理程序中拦截故障指令并重定向到 FLS（性能不切实际）

这就是为什么 `thread_safe_statics.cpp` 被排除在构建之外，并强制执行 `/Zc:threadSafeInit-` —— 编译器的魔法静态量实现也在内部依赖 `__declspec(thread)` 保护。

---

## 4. 修改模式分类

| 模式 | 文件数 | 描述 |
|---|---|---|
| **NEW** | 8 | Musa 专用文件，无基础 SDK 对应文件 |
| **GUARD** | 6 | `#if !defined NTOS_KERNEL_RUNTIME` 包装用户空间代码 |
| **SUBSTITUTE** | 4 | `__declspec(thread)` → `thread_local<T>` |
| **SIMPLIFY** | 3 | 复杂用户空间代码替换为直接内核调用 |
| **PATCH** | 10 | 微小内核模式适配（包含项、定义等） |
| **ARCH-ASM** | 16 | 架构特定汇编覆盖（ftol、CFG、处理程序） |

---

## 5. 依赖链分析

### 5.1 初始化链

```
DriverEntry (sys_main.cpp)
    ↓
__scrt_common_main_seh (sys_common.inl)
    ↓
MusaCoreStartup()                      ← Musa.Core 依赖
    ↓
TlsAlloc() → _tls_index
    ↓
__scrt_initialize_crt()
    ↓
pre_c_initializer (.CRT$XIAA)          ← initializers.cpp
    ↓
_initterm_e(__xi_a, __xi_z)            ← C 初始化器
    ↓
_initterm(__xc_a, __xc_z)              ← C++ 初始化器（静态构造函数）
    ↓
__dyn_tls_init() (tlsdyn.cpp)          ← TLS 变量初始化
    ↓
driver_main() = DriverMain()            ← 用户代码
    ↓
__scrt_common_exit → _cexit()           ← 卸载时清理
```

### 5.2 异常处理链

```
C++ throw
    ↓
__C_specific_handler (base SDK)
    ↓
__C_specific_handler_noexcept (overlay) ← 在 noexcept 违规时终止
    ↓
chandler_noexcept.cpp (overlay)
    ↓
terminate()
```

### 5.3 内存分配链

```
operator new(size)
    ↓
operator new(size, pool_type, tag)      ← knew.h
    ↓
kmalloc(size, pool, tag)                ← kmalloc.cpp
    ↓
ExAllocatePoolWithTag(pool, size, tag)  ← 内核 API
    ↓
_new_handler / _callnewh()              ← 失败时
```

---

## 6. 风险评估

| 风险 | 严重程度 | 描述 |
|---|---|---|
| **TLS 不完整** | 🔴 高 | `thread_local<T>` 模板是占位符；`thread_local` 功能标记为 TODO |
| **thread_safe_statics 排除** | 🟡 中 | 魔法静态量全局禁用；如果重新启用可能存在竞态 |
| **无文件 I/O** | 🟡 中 | `output.cpp` 覆盖层移除文件 I/O；调试输出受限 |
| **winapi_thunks 简化** | 🟢 低 | 直接内核调用有效但失去 API 集版本灵活性 |
| **区域设置禁用** | 🟢 低 | NLS 数据在内核中不可用；影响字符串转换函数 |
| **无纤程支持** | 🟢 低 | `__acrt_IsThreadAFiber()` 始终为 false；内核中不使用纤程 |

---

## 7. 版本历史指标

覆盖层包含**两个 SDK 版本**的文件，表明存在迁移历史：

| 组件 | 旧版本 | 新版本 | 状态 |
|---|---|---|---|
| MSVC | 14.43.34808 | 14.44.35207 | 活跃覆盖在新版本上 |
| UCRT | 10.0.22621.0 | 10.0.26100.0 | 活跃覆盖在新版本上 |

旧版本文件作为回滚参考和迁移辅助。
