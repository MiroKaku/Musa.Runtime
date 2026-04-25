# Musa.Runtime 开发指南

## 快速开始

### 环境要求

- Visual Studio 2022（最新版本）
- Windows Driver Kit（WDK）
- Git（支持 `--recurse-submodules`）

### 安装

#### 方法 1：NuGet 包（推荐）

右键点击项目 → **管理 NuGet 包** → 搜索 `Musa.Runtime` → 安装。

或添加到您的 `.vcxproj`：

```xml
<ItemGroup>
  <PackageReference Include="Musa.Runtime">
    <Version>0.5.1</Version>
  </PackageReference>
</ItemGroup>
```

#### 方法 2：手动导入

从 [Releases](https://github.com/MusoKaku/Musa.Runtime/releases) 下载，解压后添加到您的 `.vcxproj`：

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
<!-- above Microsoft.Cpp.targets -->
<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
```

### 第一步：重命名 DriverEntry

**Musa.Runtime 要求驱动入口点必须命名为 `DriverMain`，而不是 `DriverEntry`。**

运行时提供了自己的 `sys_main.cpp`，其中定义了 `DriverEntry` 并委托给 `DriverMain`。如果您保留自己的 `DriverEntry`，将会产生链接器冲突。

```cpp
// 正确：
EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    // 您的初始化代码
    return STATUS_SUCCESS;
}
```

---

## 在内核模式下使用 C++

### STL 容器

所有标准 STL 容器均可使用。使用 `kallocator<T>` 进行内核安全的内存分配：

```cpp
#include <unordered_map>
#include <string>
#include <vector>
#include "kext/kallocator.h"

void Example()
{
    // 使用内核池分配器的 STL 容器
    std::unordered_map<uint32_t, std::string, std::hash<uint32_t>,
        std::equal_to<uint32_t>, kallocator<std::pair<const uint32_t, std::string>>> map;

    map[1] = "hello";
    map[2] = "world";

    // 或使用带 kallocator 的 std::vector
    std::vector<int, kallocator<int>> vec;
    vec.push_back(42);
}
```

**IRQL 警告：** 默认的 `kallocator<T>` 使用 `PagedPool`。请勿在 `DISPATCH_LEVEL` 或更高 IRQL 级别使用：

```cpp
// 对于 DISPATCH_LEVEL+ 使用场景，需显式指定 NonPagedPool：
std::vector<int, kallocator<int, NonPagedPool, 'Tag1>> safeVec;
```

### 异常处理

Musa.Runtime 通过 `/EHa`（异步处理）支持 C++ 异常：

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

**限制条件：**
- 仅在 `IRQL <= APC_LEVEL` 级别工作
- 同时支持 `/EHa` 和 `/EHsc`
- 通过 SEH 集成支持内核帧的栈展开

### New/Delete

标准的 `new` 和 `delete` 操作符在内核模式下可正常工作，由 `ExAllocatePoolWithTag` 提供支持：

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

    // 数组
    auto* arr = new int[100]();
    delete[] arr;
}
```

### 静态对象

全局和静态 C++ 对象通过 CRT 初始化链（在 `sys_main.cpp` → `initialization.cpp` → 静态构造函数）在驱动加载时正确构造。

```cpp
// 此静态对象将在 DriverMain 运行之前构造
static std::string g_config = "initialized";

EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING Registry)
{
    // g_config 已经在此处构造完成
    MusaLOG("Config: %s\n", g_config.c_str());
    return STATUS_SUCCESS;
}
```

---

## 内核内存 API

### kmalloc / kfree

底层内核内存分配，支持池类型和标签控制：

```cpp
#include "kext/kmalloc.h"

void Example()
{
    // 从分页池分配
    void* ptr = kmalloc(1024, PagedPool, 'Musa');
    if (ptr) {
        // 使用 ptr...
        kfree(ptr, 'Musa');
    }

    // 零初始化分配
    void* zero = kcalloc(10, sizeof(int), NonPagedPool, 'Musa');

    // 调整大小
    void* resized = krealloc(ptr, 1024, 2048, PagedPool, 'Musa');
}
```

### kallocator 模板

用于标准容器的 STL 兼容分配器：

```cpp
#include "kext/kallocator.h"

// 默认：PagedPool，标签='RsuM'
std::vector<int, kallocator<int>> vec1;

// 显式指定池和标签
std::vector<int, kallocator<int, NonPagedPool, 'Drv1'>> vec2;

// 与 map 一起使用（注意 pair<const K, V> 类型）
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

**分配器标签约定：**
- `Musa.Veil` → Tag = `'MusV'`
- `Musa.Core` → Tag = `'MusC'`
- `Musa.Runtime` → Tag = `'RsuM'`

---

## 构建配置

### Musa.Runtime 应用的编译器设置

| 设置 | 值 | 原因 |
|---|---|---|
| 异常处理 | `/EHa` | 异步——同时捕获 C++ 异常和 SEH |
| 调用约定 | Cdecl | 标准 C 调用约定 |
| 一致性模式 | `false` | SDK 源需要非一致的 MSVC 扩展 |
| 线程安全初始化 | `/Zc:threadSafeInit-` | 禁用——内核静态初始化在加载时是单线程的 |
| 调试信息 | 程序数据库（旧式） | 与内核调试兼容 |
| 预处理器 | `_ONECORE`, `_KERNEL_MODE`, `NTOS_KERNEL_RUNTIME`, `_HAS_EXCEPTIONS` | 内核模式 + 异常支持 |

### 纯头文件模式

如果您只需要头文件（例如在单独项目中自行构建运行时）：

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>
</PropertyGroup>
```

这将设置包含路径，但**不会**自动链接 `Musa.Runtime.lib` 或 `Musa.Runtime.DriverEntry.obj`。

---

## 功能状态

| 功能 | 状态 | 备注 |
|---|---|---|
| New/Delete | ✅ 支持 | 通过 ExAllocatePoolWithTag |
| C++ 异常 | ✅ 支持 | /EHa，IRQL ≤ APC_LEVEL |
| 静态对象 | ✅ 支持 | 在驱动加载时构造 |
| SAFESEH / GS | ✅ 支持 | 缓冲区安全检查已启用 |
| STL（OneCore） | ✅ 支持 | 完整容器/算法支持 |
| STL（CoreCRT） | ✅ 支持 | 数学/IO 支持 |
| thread_local | ❌ 尚未支持 | 计划在 future release |
| /EHsc | ✅ 支持 | 同步异常处理 |
| ARM64 | ⚠️ 实验性 | 可构建但未完全测试 |

---

## 测试

`Musa.Runtime.TestForDriver` 项目展示了所有运行时功能：

```cpp
EXTERN_C NTSTATUS DriverMain(const PDRIVER_OBJECT DriverObject, const PUNICODE_STRING Registry)
{
    UNREFERENCED_PARAMETER(Registry);
    DriverObject->DriverUnload = [](auto obj) { MusaLOG("Exit."); };

    // 为测试执行扩展内核栈
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

测试在扩展的内核栈上运行，以避免深度 C++ 调用链期间发生栈溢出。

---

## 故障排除

### 链接器错误：未解析的外部符号

确保 `MusaRuntimeOnlyHeader` 设置为 `false`（或省略），以便 `Musa.Runtime.Config.targets` 链接库：

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
```

### PAGE_FAULT_IN_NONPAGED_AREA 蓝屏

您正在 `DISPATCH_LEVEL` 或更高 IRQL 级别使用 `kallocator<T>`（默认 `PagedPool`）。请指定 `NonPagedPool`：

```cpp
std::vector<int, kallocator<int, NonPagedPool, 'Tag1>> vec;
```

### 静态变量竞态条件

线程安全静态初始化已禁用（`/Zc:threadSafeInit-`）。请勿依赖 magic statics 的线程安全性。在任何并发访问之前，在 `DriverMain` 中初始化所有静态变量。

### 版本不匹配错误

构建系统会验证 overlay 和基础 UCRT 版本是否匹配。如果您看到：

```
UCRT version mismatch: overlay is X but base is Y
```

请将 `Musa.Runtime.props` 中的 `Musa_Runtime_UCRT_Version_Overlay` 和 `Musa_Runtime_UCRT_Version` 更新为相同的值。
