# [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) - 通用 C++ 运行时库

[![Actions Status](https://github.com/MiroKaku/Musa.Runtime/workflows/Build/badge.svg)](https://github.com/MiroKaku/Musa.Runtime/actions)
[![Downloads](https://img.shields.io/nuget/dt/Musa.Runtime?logo=NuGet&logoColor=blue)](https://www.nuget.org/packages/Musa.Runtime/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Runtime/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X64%7CARM64-%23FFBCD9)

* [English](https://github.com/MiroKaku/Musa.Runtime/blob/main/README.md)

## 1. 关于

Musa.Runtime 是以 [Musa.Core](https://github.com/MiroKaku/Musa.Core) 作为底层支持的微软 MSVC 运行时库，是 [ucxxrt](https://github.com/MiroKaku/ucxxrt) 的新架构的实现。

它可以让内核开发者拥有和应用开发者近似的 C++ 开发体验。

### 1.1 特性

- [x] 支持 x64、~~ARM64（实验性）~~
- [x] 支持 New/Delete
- [x] 支持 C++ Exception (/EHa、~~/EHsc~~) (IRQL <= APC_LEVEL)
- [x] 支持 Static Objects
- [x] 支持 SAFESEH、GS (Buffer Security Check)
- [x] 支持 STL (OneCore、CoreCRT)
- [ ] 支持 thread_local

### 1.2 例子

<details>

<summary>在 Musa.Runtime.TestForDriver 查看更多 ...</summary>

<pre><code>
void Test$ThrowUnknow()
{
    try {
        try {
            try {
                throw std::wstring();
            }
            catch (int& e) {
                ASSERT(false);
                MusaLOG("Catch Exception: %d\n", e);
            }
        }
        catch (std::string& e) {
            ASSERT(false);
            MusaLOG("Catch Exception: %s\n", e.c_str());
        }
    }
    catch (...) {
        MusaLOG("Catch Exception: ...\n");
    }
}

void Test$HashMap()
{
    auto Rand = std::mt19937_64(::rand());
    auto Map  = std::unordered_map<uint32_t, std::string>();
    for (auto i = 0u; i < 10; ++i) {
        Map[i] = std::to_string(Rand());
    }

    for (const auto& Item : Map) {
        MusaLOG("map[%ld] = %s\n", Item.first, Item.second.c_str());
    }
}
</code></pre>

</details>

## 2. 使用方法

** 首先 `DriverEntry` 需要改为 `DriverMain`。**

### 2.1 方法一（推荐）

右键单击该项目并选择“管理 NuGet 包”，然后搜索`Musa.Runtime`并选择适合你的版本，最后单击“安装”。

或者

如果你的项目模板用的是 [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows)，那么可以直接在你的 `.vcxproj` 文件里面添加下面代码：

```XML
  <ItemGroup>
    <PackageReference Include="Musa.Runtime">
      <!-- 期望的版本 -->
      <Version>0.1.0</Version>
    </PackageReference>
  </ItemGroup>
```

### 2.2 方法二

1. 从 [release](https://github.com/MiroKaku/Musa.Runtime/releases) 下载最新包并解压。
2. 编辑你的 `.vcxproj` 文件，在里面添加下面代码：

```XML
  <PropertyGroup>
    <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
  </PropertyGroup>
  <Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
  <Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
  <!-- 在这一行上面 -->
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
```

### 2.3 仅头文件模式

在你的 `.vcxproj` 文件里面添加下面代码：

```XML
  <PropertyGroup>
    <MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>
  </PropertyGroup>
```

这个模式不会自动引入lib文件。

## 3. 编译方法

IDE：Visual Studio 2022 最新版

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> .\BuildAllTargets.cmd
```

## 4. 鸣谢

> [IntelliJ IDEA](https://zh.wikipedia.org/zh-hans/IntelliJ_IDEA) 是一个在各个方面都最大程度地提高开发人员的生产力的 IDE。

特别感谢 [JetBrains](https://www.jetbrains.com/?from=meesong) 为开源项目提供免费的 [Resharper C++](https://www.jetbrains.com/resharper-cpp/?from=meesong) 等 IDE 的授权

[<img src="https://resources.jetbrains.com/storage/products/company/brand/logos/ReSharperCPP_icon.png" alt="ReSharper C++ logo." width=200>](https://www.jetbrains.com/?from=meesong)

## 5. 引用参考

* [Microsoft's C++ Standard Library](https://github.com/microsoft/stl)
* [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)
* [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL)
* [msvcr14x](https://github.com/sonyps5201314/msvcr14x)

> 非常感谢这些优秀的项目，没有它们的存在，就不会有 Musa.Runtime。
