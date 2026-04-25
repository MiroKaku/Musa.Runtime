# [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) - 通用 C++ 运行时库

[![Actions Status](https://github.com/MiroKaku/Musa.Runtime/workflows/CI/badge.svg)](https://github.com/MiroKaku/Musa.Runtime/actions)
[![Downloads](https://img.shields.io/nuget/dt/Musa.Runtime?logo=NuGet&logoColor=blue)](https://www.nuget.org/packages/Musa.Runtime/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Runtime/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X64%7CARM64-%23FFBCD9)

* [English](README.md) · [简体中文](README.zh-CN.md)
* 📖 [架构文档](docs/ARCHITECTURE.zh-CN.md) · [开发指南](docs/DEVELOPER-GUIDE.zh-CN.md) · [构建系统](docs/BUILD-SYSTEM.zh-CN.md) · [Overlay 差异报告](docs/OVERLAY-DIFF-REPORT.zh-CN.md)

---

## 概述

Musa.Runtime 是一个面向 Windows 内核模式开发的 MSVC 运行时库，基于 [Musa.Core](https://github.com/MiroKaku/Musa.Core) 构建，继承并发展了 [ucxxrt](https://github.com/MiroKaku/ucxxrt) 的架构设计。

**核心目标：** 让内核开发者获得与应用程序开发者一致的 C++ 开发体验。

**设计亮点：** 借鉴 Docker 分层文件系统思想，通过 overlay 机制在原版 MSVC SDK 源码之上进行选择性替换，避免 fork 维护独立副本。详见 [架构文档](docs/ARCHITECTURE.zh-CN.md)。

## 快速开始

### 1. 安装

**NuGet（推荐）：**

```xml
<ItemGroup>
  <PackageReference Include="Musa.Runtime">
    <Version>0.5.1</Version>
  </PackageReference>
</ItemGroup>
```

或右键项目 → **管理 NuGet 程序包** → 搜索 `Musa.Runtime` → 安装。

**手动导入：** 从 [Releases](https://github.com/MiroKaku/Musa.Runtime/releases) 下载后解压：

```xml
<PropertyGroup>
  <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
</PropertyGroup>
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
<Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
<!-- 放在 Microsoft.Cpp.targets 之前 -->
```

### 2. 将 `DriverEntry` 重命名为 `DriverMain`

Musa.Runtime 自带 `DriverEntry` 入口点（负责 CRT 初始化），你需要将驱动入口重命名为 `DriverMain`：

```cpp
EXTERN_C NTSTATUS DriverMain(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DriverObject->DriverUnload = [](auto obj) { MusaLOG("Unload."); };
    // 你的代码
    return STATUS_SUCCESS;
}
```

### 3. 在内核中使用 C++

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

> ⚠️ `kallocator` 默认使用 `PagedPool`。在 `DISPATCH_LEVEL` 及以上 IRQL 使用时需指定 `NonPagedPool`。

## 功能特性

| 特性 | 状态 | 备注 |
|---|---|---|
| x64 | ✅ | 主力平台 |
| ARM64 | ⚠️ | 实验性支持 |
| New / Delete | ✅ | 基于 `ExAllocatePoolWithTag` |
| C++ 异常（`/EHa`、`/EHsc`） | ✅ | IRQL ≤ APC_LEVEL |
| 静态对象构造/析构 | ✅ | 驱动加载时自动执行 |
| SAFESEH / GS | ✅ | 缓冲区安全检查 |
| STL（OneCore / CoreCRT） | ✅ | 完整容器/算法支持 |
| `thread_local` | ❌ | 编译器层面限制（fs/gs 寄存器访问） |

## 文档索引

| 文档 | 内容 |
|---|---|
| [📐 架构文档](docs/ARCHITECTURE.zh-CN.md) | 设计理念（Overlay 模式）、组件架构、内核适配机制 |
| [📝 开发指南](docs/DEVELOPER-GUIDE.zh-CN.md) | 安装使用、STL 容器、异常处理、kallocator、故障排查 |
| [🔧 构建系统](docs/BUILD-SYSTEM.zh-CN.md) | 项目结构、构建流程、版本管理、发布管道 |
| [🔬 Overlay 差异报告](docs/OVERLAY-DIFF-REPORT.zh-CN.md) | 所有 overlay 文件的逐文件差异分析与归类 |

## 从源码构建

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> cd Musa.Runtime
> .\BuildAllTargets.cmd
```

前置要求：Visual Studio 2026（最新版）+ WDK

## 参考项目

- [Microsoft's C++ Standard Library](https://github.com/microsoft/stl)
- [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)
- [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL)
- [msvcr14x](https://github.com/sonyps5201314/msvcr14x)
- [Boost.Math](https://www.boost.org/doc/libs/release/libs/math/)
