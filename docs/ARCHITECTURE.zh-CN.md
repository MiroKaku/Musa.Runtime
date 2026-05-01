# Musa.Runtime 架构

## 概述

Musa.Runtime 是为 Windows 内核模式开发而适配的 Microsoft MSVC 运行时库。它为内核开发者提供与用户态应用开发几乎相同的 C++ 体验——STL 容器、异常、new/delete、静态对象，以及更多——全部运行在内核 IRQL。

该项目基于 [Musa.Core](https://github.com/MiroKaku/Musa-Core) 构建，并实现了 [ucxxrt](https://github.com/MiroKaku/ucxxrt) 中探索的架构。

**支持的平台：** x64（稳定），ARM64（实验性），Win32（有限）
**目标 SDK 版本：** MSVC 14.44.35207 / UCRT 10.0.26100.0

---

## 设计理念：覆盖文件系统模式

Musa.Runtime 采用 Docker 的分层文件系统概念来实现选择性代码覆盖。它不是 fork 并维护一份独立的 MSVC 运行时源码，而是在原始 SDK 源码树之上层叠 Musa 特定的修改。

```
┌─────────────────────────────────────────────────────┐
│                    Consumer Project                 │
│              (Kernel Driver / KMDF)                 │
├─────────────────────────────────────────────────────┤
│              Musa.Runtime.lib (aggregate)           │
│  ┌────────┬────────┬────────┬────────┬──────────┐   │
│  │  CRT   │  STL   │ VCRT   │  UCRT  │   Math   │   │
│  │ .lib   │ .lib   │ .lib   │ .lib   │   .lib   │   │
│  └───┬────┴───┬────┴───┬────┴───┬────┴────┬─────┘   │
│      │ overlay│ overlay│ overlay│  overlay│ extract │
├──────┼────────┼────────┼────────┼─────────┼─────────┤
│  ┌───┴────────┴──┐  ┌──┴────────┴───┐               │
│  │ Musa.Runtime/ │  │ Musa.Runtime/ │               │
│  │  MSVC/overlay │  │  UCRT/overlay │  ← TOP LAYER  │
│  └───────┬───────┘  └───────┬───────┘               │
│          │                  │                       │
│  ┌───────┴──────────────────┴───────┐               │
│  │  Microsoft.VisualC.Runtime/      │               │
│  │    MSVC/14.44.35207/crt/         │               │
│  │    UCRT/10.0.26100.0/ucrt/       │  ← BASE LAYER │
│  └──────────────────────────────────┘               │
└─────────────────────────────────────────────────────┘
```

### 层机制

**基础层**（`Microsoft.VisualC.Runtime/`）—— 原始 MSVC SDK 源码的未修改副本，按版本组织：
- `MSVC/14.43.34808/` —— 之前的 VC 运行时版本
- `MSVC/14.44.35207/` —— 当前的 VC 运行时版本
- `UCRT/10.0.22621.0/` —— 之前的 UCRT 版本
- `UCRT/10.0.26100.0/` —— 当前的 UCRT 版本

**顶层**（`Musa.Runtime/MSVC/` 和 `Musa.Runtime/UCRT/`）—— Musa 特定的覆盖，替换或扩展基础 SDK 文件。覆盖层镜像基础目录结构（`crt/vcruntime/`、`ucrt/heap/` 等），以便路径解析自然地优先选择 Musa 版本。

### 包含路径解析

覆盖通过 `Musa.Runtime.props` 中的 MSBuild 包含路径顺序实现：

```xml
<AdditionalIncludeDirectories>
  $(Musa_Runtime_VC_IncludePath_Overlay);    <!-- Musa overrides FIRST -->
  $(Musa_Runtime_UCRT_IncludePath_Overlay);
  $(Musa_Runtime_VC_IncludePath);            <!-- Base SDK SECOND -->
  $(Musa_Runtime_UCRT_IncludePath);
  %(AdditionalIncludeDirectories);
</AdditionalIncludeDirectories>
```

`$(Musa_Runtime_VC_ToolsInstallDir_Overlay)` 指向的文件来自顶层；`$(Musa_Runtime_VC_ToolsInstallDir)` 指向的文件来自基础层。每个 `.vcxproj` 明确选择每个源文件来自哪一层。

### 版本一致性

构建系统验证覆盖和基础 SDK 版本完全匹配：

```xml
<Target Name="ValidateMusaRuntimeVersions" BeforeTargets="PrepareForBuild">
  <Error Condition="'$(Musa_Runtime_UCRT_Version_Overlay)' != '$(Musa_Runtime_UCRT_Version)'"
         Text="UCRT version mismatch: overlay is $(Musa_Runtime_UCRT_Version_Overlay) but base is $(Musa_Runtime_UCRT_Version)." />
</Target>
```

这可以防止 ABI 不匹配——如果覆盖代码使用与基础源码不同版本的 SDK 头文件编译，就会发生这种不匹配。

---

## 组件架构

Musa.Runtime 由五个子库组成，每个子库负责 C/C++ 运行时的不同部分：

### 1. Musa.Runtime.CRT

**角色：** C 运行时支持——new/delete、SEH 前序/后序、GS 安全检查、线程本地存储初始化、静态对象构造函数。

**关键源文件：**
- `kext/new_km.cpp`、`kext/delete_km.cpp` —— 使用 `ExAllocatePoolWithTag`/`ExFreePoolWithTag` 的内核模式 new/delete 实现
- `$(BaseSDK)/crt/vcruntime/` —— 异常处理（ehvccctr、ehvecctr、ehvecdtr）、delete 运算符、fltused、GS report、guard 支持、TLSSUP
- `$(Overlay)/crt/vcruntime/` —— CPU 分发、初始化器、sys_main（DriverEntry 重命名）、sys_dllmain、TLS dtor、工具函数
- 架构特定汇编：i386（ftol 变体）、x64（guard_dispatch、guard_xfg_dispatch）、ARM64（chkstk、secpushpop、guard_dispatch）

**输出：** `Musa.Runtime.CRT.lib`
**特别之处：** 将 `sys_main.obj` 发布为 `Musa.Runtime.DriverEntry.obj`，供消费者的链接器选用。

### 2. Musa.Runtime.STL

**角色：** 标准模板库实现——容器、算法、iostreams 支持、异常指针、futures、线程原语。

**关键源文件：**
- `$(BaseSDK)/crt/stl/` —— 约 70 个 STL 实现文件：atomic、charconv、异常指针、futures、memory resource、mutex 操作、math 函数（xdint、xdnorm 等）、线程支持
- `$(Overlay)/crt/stl/` —— mutex.cpp、syserror_import_lib.cpp、xrngabort.cpp（Musa 特定适配）
- 排除于构建：`special_math.cpp`、`xcosh.cpp`、`fcosh.cpp`、`fsinh.cpp`、`lcosh.cpp`、`lsinh.cpp`、`xsinh.cpp`（这些由 Boost.Math 替换）

**输出：** `Musa.Runtime.STL.lib`

### 3. Musa.Runtime.VCRT

**角色：** Visual C++ 运行时——异常处理基础设施、RTTI、帧管理、类型信息、unexpected/uncaught 异常。

**关键源文件：**
- `$(BaseSDK)/crt/vcruntime/` —— ehhelpers、ehstate、frame、locks、per_thread_data、purevirt、rtti、std_exception、std_type_info、throw、uncaught_exception(s)、unexpected、user
- `$(Overlay)/crt/vcruntime/` —— chandler_noexcept、initialization、jbcxrval.c、undname.cxx
- 架构特定：i386（chandler4.c、trnsctrl.cpp、exsup4.asm）、x64（handlers.asm、notify.asm）、ARM64（handlers.asm、handler.asm、notify.asm、riscchandler.cpp、risctrnsctrl.cpp）

**输出：** `Musa.Runtime.VCRT.lib`

### 4. Musa.Runtime.UCRT

**角色：** 通用 C 运行时——堆分配、启动/退出、locale、stdlib、信号处理、内部初始化。

**关键源文件：**
- `kext/kmalloc.cpp`、`kext/kfree.cpp` —— 内核内存分配层（`kmalloc`、`kfree`、`kcalloc`、`krealloc`、`krecalloc`）使用 Windows 内核池 API
- `$(BaseSDK)/ucrt/heap/` —— malloc、calloc、free、realloc、recalloc、msize、new_handler、new_mode
- `$(BaseSDK)/ucrt/` —— startup（abort、assert、exit、initterm、onexit）、stdlib（abs、div、qsort、bsearch、rand 等）、initializers
- `$(Overlay)/ucrt/` —— convert（mbstwoombs、wcstombs、_ctype）、heap（debug_heap）、internal（initialization、locks、OutputDebugStringA、per_thread_data、winapi_thunks）、locale（locale_update、nlsdata、wsetlocale）、misc（dbgrptt、exception_filter、signal）、startup（thread）、stdio（output）

**输出：** `Musa.Runtime.UCRT.lib`

### 5. Musa.Runtime.Math

**角色：** 从 UCRT 静态库提取的数学库，包含 Musa 特定的排除项。

**构建过程：** 与其他四个库编译单个源文件不同，Math 通过使用 `lib.exe /REMOVE` 从 UCRT 库提取特定目标文件来构建：

```
Build.Microsoft.VisualC.Library.bat $(PlatformTarget) \
  -lib=libucrt[d].lib \
  -out=$(TargetPath) \
  -exclusion_list=Musa.Runtime.Math.Exclusion.txt
```

排除文件列出了要从提取的库中移除的目标文件。这种方法确保了数学函数与 UCRT 的一致性，同时允许选择性移除内核不兼容的代码。

**输出：** `Musa.Runtime.Math.lib`

### 聚合库

**Musa.Runtime.vcxproj** 将五个子库链接成单一的 `Musa.Runtime.lib`：

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

聚合库是消费者链接的目标。

---

## 内核模式适配

### 内存分配

标准堆分配器被替换为 Windows 内核池分配：

```
用户态：  malloc → HeapAlloc → Windows Heap
内核态：  kmalloc → ExAllocatePoolWithTag → Kernel Pool
```

`kallocator<T>` 模板提供了一个与 STL 兼容的分配器，使用内核池：

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

**警告：** 默认池类型是 `PagedPool`。不要在 `DISPATCH_LEVEL` 或更高 IRQL 下使用 `kallocator<T>` 而不指定 `NonPagedPool`——在 elevated IRQL 下访问分页内存会导致 `PAGE_FAULT_IN_NONPAGED_AREA` 蓝屏。

### 异常处理

- 模式：`/EHa`（异步异常处理）—— 支持 C++ 异常和 SEH
- IRQL 约束：异常处理在 `IRQL <= APC_LEVEL` 下工作
- 线程安全静态变量：**禁用**（`/Zc:threadSafeInit-`）—— 静态局部变量的并发初始化可能有竞争

### 关键预处理器定义

| 符号 | 用途 |
|---|---|
| `_ONECORE` | OneCore 目标（减少的 API 表面） |
| `_KERNEL_MODE` / `NTOS_KERNEL_RUNTIME` | 内核模式编译 |
| `_HAS_EXCEPTIONS` | 启用 C++ 异常 |
| `_CRTBLD` / `_VCRT_BUILD` / `_VCRT_ENCLAVE_BUILD` | 运行时构建模式 |
| `_STATIC_CPPLIB` | 静态 STL 链接 |
| `DisableKernelFlag=true` | 覆盖 WDK 的默认纯 C 模式 |

### 线程本地存储

- `thread_local` **尚未支持**（功能中列为 TODO）
- 静态 TLS 初始化使用覆盖层的 `dyn_tls_init.c`、`tlsdtor.cpp`
- 动态 TLS（`tlsdyn.cpp`）和线程安全静态变量排除于构建

---

## 构建系统

### 项目依赖

```
Musa.Runtime.vcxproj
├── Musa.Runtime.CRT.vcxproj
├── Musa.Runtime.STL.vcxproj
├── Musa.Runtime.VCRT.vcxproj
├── Musa.Runtime.UCRT.vcxproj
└── Musa.Runtime.Math.vcxproj
```

### 构建入口点

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> .\BuildAllTargets.cmd
```

`BuildAllTargets.cmd`：
1. 删除 `Output/` 以进行全新构建
2. 初始化 Visual Studio 环境（调用 `InitializeVisualStudioEnvironment.cmd`）
3. 使用二进制日志调用 MSBuild 构建 `BuildAllTargets.proj`

### 构建平台支持

- **x64** —— 完全支持，主要目标
- **ARM64** —— 支持（实验性）
- **Win32** —— 部分支持（大多数项目中的平台 props 被注释掉了）

### SDK 版本配置

`Musa.Runtime.props` 中定义的当前版本：
- `Musa_Runtime_VC_Version_Overlay` = `14.44.35207`
- `Musa_Runtime_UCRT_Version_Overlay` = `10.0.26100.0`

基础 SDK 中也提供之前的版本以供比较或回滚。

---

## NuGet 包结构

NuGet 包（`Musa.Runtime.nuspec`）分发：
- `build/native/Musa.Runtime.props` —— 消费者的构建配置
- `build/native/Musa.Runtime.Config.props` —— 包含/库路径设置
- `build/native/Musa.Runtime.Config.targets` —— 链接器依赖（`Musa.Runtime.lib`、`Musa.Runtime.DriverEntry.obj`）
- `build/native/Include/` —— 来自 `kext/` 的公共头文件
- `build/native/Library/{Configuration}/{Platform}/` —— 编译的 `.lib` 文件

**依赖：** `Musa.Core`（版本 >= 0.5.1）

### 纯头文件模式

在消费者的 `.vcxproj` 中设置 `<MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>` 可禁用自动 lib 导入，适用于只需要头文件定义而不链接运行时的项目。

---

## 参考项目

- [Microsoft's C++ Standard Library (STL)](https://github.com/microsoft/stl) —— CRT/STL/VCRT 代码来源
- [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5) —— 轻量级 C 运行时方法
- [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL) —— 覆盖技术参考
- [msvcr14x](https://github.com/sonyps5201314/msvcr14x) —— MSVC 运行时适配参考
- [Boost.Math](https://www.boost.org/doc/libs/release/libs/math/) —— 特殊数学函数的子模块
