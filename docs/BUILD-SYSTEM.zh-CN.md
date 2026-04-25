# Musa.Runtime 构建系统参考

## 项目结构

```
Musa.Runtime/
├── BuildAllTargets.cmd              # 顶层构建入口点
├── BuildAllTargets.proj             # MSBuild 项目文件（targets）
├── InitializeVisualStudioEnvironment.cmd  # VS 环境设置
├── Directory.Build.props            # 根 MSBuild 属性
├── Directory.Build.targets          # 根 MSBuild 目标
├── Directory.Packages.Cpp.props     # 包管理
├── global.json                      # SDK 版本锁定
│
├── Microsoft.VisualC.Runtime/       # 基础 SDK 层（未修改的副本）
│   ├── MSVC/
│   │   ├── 14.43.34808/            # 之前的 VC 运行时
│   │   └── 14.44.35207/            # 当前的 VC 运行时
│   │       ├── crt/
│   │       │   ├── i386/           # x86 汇编和 C
│   │       │   ├── arm64/          # ARM64 汇编
│   │       │   ├── x64/            # x64 汇编
│   │       │   ├── vcruntime/      # VC 运行时源码
│   │       │   └── stl/            # STL 源码
│   │       └── include/            # 公共头文件
│   └── UCRT/
│       └── 10.0.26100.0/
│           └── ucrt/               # 通用 CRT 源码
│
├── Musa.Runtime/                    # Overlay 层 + 主项目
│   ├── Musa.Runtime.props           # 版本和路径配置
│   ├── Musa.Runtime.vcxproj         # 聚合库
│   ├── Musa.Runtime.CRT.vcxproj     # CRT 子库
│   ├── Musa.Runtime.STL.vcxproj     # STL 子库
│   ├── Musa.Runtime.VCRT.vcxproj    # VCRT 子库
│   ├── Musa.Runtime.UCRT.vcxproj    # UCRT 子库
│   ├── Musa.Runtime.Math.vcxproj    # Math 子库
│   ├── Build.Microsoft.VisualC.Library.bat  # Math 提取脚本
│   ├── Musa.Runtime.Math.Exclusion.txt      # Math 中要排除的对象
│   ├── universal.h                  # 共享预编译头
│   ├── universal.cpp                # 共享预编译源
│   ├── kext/                        # 内核模式扩展
│   │   ├── kallocator.h             # STL 兼容的内核分配器
│   │   ├── knew.h                   # Operator new 声明
│   │   ├── kmalloc.h                # kmalloc/kfree API
│   │   ├── kmalloc.cpp              # kmalloc 实现
│   │   ├── kfree.cpp                # kfree 实现
│   │   ├── new_km.cpp               # 内核模式 new
│   │   ├── delete_km.cpp            # 内核模式 delete
│   │   └── thread_local.h           # 线程本地存储（未完成）
│   ├── MSVC/                        # VC overlay 文件
│   │   └── 14.44.35207/crt/         # 覆盖的 VC 源码
│   └── UCRT/                        # UCRT overlay 文件
│       └── 10.0.26100.0/ucrt/       # 覆盖的 UCRT 源码
│
├── Musa.Runtime.NuGet/              # NuGet 打包
│   ├── Musa.Runtime.nuspec          # 包清单
│   ├── Musa.Runtime.props           # NuGet 构建属性
│   ├── Musa.Runtime.Config.props    # 使用者的 include/lib 路径
│   └── Musa.Runtime.Config.targets  # 使用者的链接器依赖
│
├── Musa.Runtime.TestForDriver/      # 测试驱动
│   ├── TestForDriver.Main.cpp       # DriverMain 入口点
│   ├── Universal.h / Universal.cpp  # 测试预编译头
│   └── Musa.Tests/                  # 单元测试定义
│
├── Boost.Math/                      # 数学库子模块
│   └── include/boost/math/          # 特殊数学函数
│
├── Output/                          # 构建产物
└── Publish/                         # 分发输出
```

## 构建流程

### 步骤说明

1. **清理：** `BuildAllTargets.cmd` 删除 `Output/` 目录
2. **环境：** 通过 `InitializeVisualStudioEnvironment.cmd` 初始化 Visual Studio 构建环境
3. **MSBuild：** 运行 `MSBuild -binaryLogger:Output\BuildAllTargets.binlog -m BuildAllTargets.proj`
4. **并行构建：** `-m` 标志启用跨子项目的并行编译
5. **发布：** 构建后目标将产物复制到 `Publish/`

### 子项目构建顺序

来自 `Musa.Runtime.slnx`：

```
Musa.Runtime.CRT.vcxproj     ─┐
Musa.Runtime.STL.vcxproj     ─┤
Musa.Runtime.VCRT.vcxproj    ─┼──→ Musa.Runtime.vcxproj (聚合)
Musa.Runtime.UCRT.vcxproj    ─┤
Musa.Runtime.Math.vcxproj    ─┘
```

聚合库（`Musa.Runtime.vcxproj`）对所有五个子库有构建依赖，并将它们链接在一起。

### 预编译头

除 Math 外的所有项目都使用共享预编译头：

- **头文件：** `universal.h`
- **源文件：** `universal.cpp`（创建 PCH）
- **强制包含：** 所有编译单元自动包含 `universal.h`

`universal.h` 设置以下内容：

- `_NO_CRT_STDIO_INLINE` — 禁用内联 stdio（NLS 待办）
- `_CRT_SECURE_NO_WARNINGS` — 抑制安全警告
- `time_t` 类型定义（基于 `_USE_32BIT_TIME_T` 的 32/64 位）
- `<Veil.h>` 包含 — 核心系统头
- 警告抑制：4005, 4189, 4245, 4457, 4499, 4838

### 平台支持

| 平台 | 状态 | 备注 |
|---|---|---|
| x64 | ✅ 完整 | 主要开发目标 |
| ARM64 | ⚠️ 部分 | 文件存在，部分已排除 |
| Win32 | ❌ 禁用 | 平台属性已注释 |

ARM64 特定文件：

- 用于异常处理器的 `MARMASM` 汇编文件
- VCRT 中的 `riscchandler.cpp`、`risctrnsctrl.cpp`
- 通过 `ExcludedFromBuild` 条件进行平台特定排除

### 数学库构建

数学库使用独特的构建方法——从 UCRT 库中提取对象，而不是编译源文件：

```
1. 构建 libucrt[d].lib（来自 UCRT 项目）
2. 运行 Build.Microsoft.VisualC.Library.bat：
   lib libucrt[d].lib /NOLOGO /OUT=Musa.Runtime.Math.lib
3. 移除排除的对象：
   for each obj in Musa.Runtime.Math.Exclusion.txt:
     lib Musa.Runtime.Math.lib /REMOVE:obj /IGNORE:4006,4014
```

这确保了与 UCRT 的数学一致性，同时允许移除与内核不兼容的对象。

## 版本管理

### 当前版本

| 组件 | 版本 | 位置 |
|---|---|---|
| VC 运行时 | 14.44.35207 | `MSVC/14.44.35207/` |
| UCRT | 10.0.26100.0 | `UCRT/10.0.26100.0/` |

### 版本属性

在 `Musa.Runtime/Musa.Runtime.props` 中定义：

```xml
<!-- Overlay（顶层）版本 -->
<Musa_Runtime_VC_Version_Overlay>14.44.35207</Musa_Runtime_VC_Version_Overlay>
<Musa_Runtime_UCRT_Version_Overlay>10.0.26100.0</Musa_Runtime_UCRT_Version_Overlay>

<!-- Base（底层）版本 -->
<Musa_Runtime_VC_Version>14.44.35207</Musa_Runtime_VC_Version>
<Musa_Runtime_UCRT_Version>10.0.26100.0</Musa_Runtime_UCRT_Version>
```

### 路径解析

```
$(Musa_Runtime_VC_ToolsInstallDir_Overlay)
  → Musa.Runtime/MSVC/14.44.35207/

$(Musa_Runtime_VC_ToolsInstallDir)
  → Microsoft.VisualC.Runtime/MSVC/14.44.35207/

$(Musa_Runtime_UCRT_ToolsInstallDir_Overlay)
  → Musa.Runtime/UCRT/10.0.26100.0/

$(Musa_Runtime_UCRT_ToolsInstallDir)
  → Microsoft.VisualC.Runtime/UCRT/10.0.26100.0/
```

### 更新 SDK 版本

要更新到更新的 MSVC/UCRT 版本：

1. 将新 SDK 源码放入 `Microsoft.VisualC.Runtime/MSVC/<version>/` 和 `UCRT/<version>/`
2. 在 `Musa.Runtime/MSVC/<version>/` 和 `UCRT/<version>/` 创建 overlay 目录
3. 在 `Musa.Runtime.props` 中更新版本属性
4. 从之前的版本复制/合并 overlay 文件（逐一检查兼容性）
5. 运行构建并修复任何编译问题

## 发布管道

成功构建后，产物将发布到 `Publish/`：

```
Publish/
├── LICENSE                      # MIT 许可证
├── README.md                    # 项目自述文件
├── Include/
│   ├── kallocator.h             # 公共：STL 分配器
│   ├── knew.h                   # 公共：operator new
│   ├── thread_local.h           # 公共：线程本地（未完成）
│   └── kmalloc.h                # 公共：kmalloc API
└── Library/
    ├── {Configuration}\         # Debug 或 Release
    │   └── {Platform}\          # x64 或 ARM64
    │       ├── Musa.Runtime.lib # 聚合库
    │       └── Musa.Runtime.DriverEntry.obj  # DriverEntry 存根
```

`Musa.Runtime.DriverEntry.obj` 来自 `Musa.Runtime.CRT.vcxproj` 的 `sys_main.obj` 输出。此对象提供 `DriverEntry` → `DriverMain` 的委托。

## 构建自定义

### Mile.Project.Configurations

Musa.Runtime 使用 [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows) 构建框架：

- `Mile.Project.Platform.x64.props` / `ARM64.props` — 平台配置
- `Mile.Project.Cpp.Default.props` — 默认 C++ 设置
- `Mile.Project.Cpp.props` / `Mile.Project.Cpp.targets` — C++ 构建逻辑
- `marmasm.props` / `marmasm.targets` — ARM64 汇编支持

关键属性：

- `MileProjectType` = `StaticLibrary`
- `MileProjectUseKernelMode` = `true`
- `MileProjectUseWindowsDriverKit` = `true`
- `MileProjectOutputPath` = `$(MSBuildThisFileDirectory)Output/`

### 编译器标志

通过 `Musa.Runtime.props` 应用：

```xml
<AdditionalOptions>/Zc:__cplusplus /Zc:threadSafeInit-</AdditionalOptions>
<ExceptionHandling>Async</ExceptionHandling>
<CallingConvention>Cdecl</CallingConvention>
<ConformanceMode>false</ConformanceMode>
<MultiProcessorCompilation>true</MultiProcessorCompilation>
<WholeProgramOptimization>false</WholeProgramOptimization>
<DebugInformationFormat>OldStyle</DebugInformationFormat>
```

### MASM 设置

x86 汇编文件使用 SAFESEH：

```xml
<UseSafeExceptionHandlers>true</UseSafeExceptionHandlers>
```

## 构建输出产物

| 产物 | 源项目 | 用途 |
|---|---|---|
| `Musa.Runtime.CRT.lib` | CRT.vcxproj | C 运行时、new/delete、SEH、GS |
| `Musa.Runtime.STL.lib` | STL.vcxproj | 标准模板库 |
| `Musa.Runtime.VCRT.lib` | VCRT.vcxproj | VC 运行时、异常、RTTI |
| `Musa.Runtime.UCRT.lib` | UCRT.vcxproj | 通用 CRT、堆、stdlib |
| `Musa.Runtime.Math.lib` | Math.vcxproj | 数学函数（从 UCRT 提取） |
| `Musa.Runtime.lib` | Musa.Runtime.vcxproj | 上述所有聚合 |
| `Musa.Runtime.DriverEntry.obj` | CRT.vcxproj (sys_main.obj) | DriverEntry → DriverMain 委托 |
| `BuildAllTargets.binlog` | BuildAllTargets.cmd | 用于诊断的二进制 MSBuild 日志 |
