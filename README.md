# [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) - Universal C++ RunTime Library

[![Actions Status](https://github.com/MiroKaku/Musa.Runtime/workflows/Build/badge.svg)](https://github.com/MiroKaku/Musa.Runtime/actions)
[![Downloads](https://img.shields.io/nuget/dt/Musa.Runtime?logo=NuGet&logoColor=blue)](https://www.nuget.org/packages/Musa.Runtime/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Runtime/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X64%7CARM64-%23FFBCD9)

* [简体中文](https://github.com/MiroKaku/Musa.Runtime/blob/main/README.zh-CN.md)

## 1. About
Musa.Runtime is a Microsoft MSVC runtime library based on [Musa.Core](https://github.com/MiroKaku/Musa.Core) and is an implementation of the new architecture of [ucxxrt](https://github.com/MiroKaku/ucxxrt).

It allows kernel developers to have a C++ development experience similar to that of application developers.

### 1.1 Features

- [x] support x64、~~ARM64 (Experimental)~~
- [x] support New/Delete
- [x] support C++ Exception (/EHa、~~/EHsc~~) (IRQL <= APC_LEVEL)
- [x] support Static Objects
- [x] support SAFESEH、GS (Buffer Security Check)
- [x] support STL (OneCore、CoreCRT)
- [ ] support thread_local

### 1.2 Example

<details>

<summary>See Musa.Runtime.TestForDriver for more information...</summary>

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

## 2. How to use

**First, rename `DriverEntry` to `DriverMain`.**

### 2.1 Method 1 (recommended)

Right click on the project, select "Manage NuGet Packages".
Search for `Musa.Runtime`, choose the version that suits you, and then click "Install".

Or

If your project template uses [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows), you can add the following code directly to your `.vcxproj` file:

```XML
  <ItemGroup>
    <PackageReference Include="Musa.Runtime">
      <!-- Expected version -->
      <Version>0.1.0</Version>
    </PackageReference>
  </ItemGroup>
```

### 2.2 Method 2

1. Download the latest package from [Releases](https://github.com/MiroKaku/Musa.Runtime/releases) and unzip it.
2. Edit your `.vcxproj` file and add the following code:

```XML
  <PropertyGroup>
    <MusaRuntimeOnlyHeader>false</MusaRuntimeOnlyHeader>
  </PropertyGroup>
  <Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.props" />
  <Import Project="..\Musa.Runtime\config\Musa.Runtime.Config.targets" />
  <!-- above this row -->
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
```

### 2.3 Header-only mode

Add the following code to your `.vcxproj` file:

```XML
  <PropertyGroup>
    <MusaRuntimeOnlyHeader>true</MusaRuntimeOnlyHeader>
  </PropertyGroup>
```

This mode will not automatically import lib files.

## 3. How to build

IDE：Visual Studio 2022 latest version

```cmd
> git clone --recurse-submodules https://github.com/MiroKaku/Musa.Runtime.git
> .\BuildAllTargets.cmd
```

## 4. Acknowledgements

Thanks to [JetBrains](https://www.jetbrains.com/?from=meesong) for providing free licenses such as [Resharper C++](https://www.jetbrains.com/resharper-cpp/?from=meesong) for my open-source projects.

[<img src="https://resources.jetbrains.com/storage/products/company/brand/logos/ReSharperCPP_icon.png" alt="ReSharper C++ logo." width=200>](https://www.jetbrains.com/?from=meesong)

## 5. References

* [Microsoft's C++ Standard Library](https://github.com/microsoft/stl)
* [Chuyu-Team/VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)
* [RetrievAL](https://github.com/SpoilerScriptsGroup/RetrievAL)
* [msvcr14x](https://github.com/sonyps5201314/msvcr14x)

> Great thanks to these excellent projects. Without their existence, there would be no Musa.Runtime then.
