# Musa.Runtime Build System Reference

## Project Structure

```
Musa.Runtime/
├── BuildAllTargets.cmd              # Top-level build entry point
├── BuildAllTargets.proj             # MSBuild project file (targets)
├── InitializeVisualStudioEnvironment.cmd  # VS environment setup
├── Directory.Build.props            # Root MSBuild properties
├── Directory.Build.targets          # Root MSBuild targets
├── Directory.Packages.Cpp.props     # Package management
├── global.json                      # SDK version pin
│
├── Microsoft.VisualC.Runtime/       # Base SDK layer (unmodified copy)
│   ├── MSVC/
│   │   ├── 14.43.34808/            # Previous VC runtime
│   │   └── 14.44.35207/            # Current VC runtime
│   │       ├── crt/
│   │       │   ├── i386/           # x86 assembly & C
│   │       │   ├── arm64/          # ARM64 assembly
│   │       │   ├── x64/            # x64 assembly
│   │       │   ├── vcruntime/      # VC runtime source
│   │       │   └── stl/            # STL source
│   │       └── include/            # Public headers
│   └── UCRT/
│       └── 10.0.26100.0/
│           └── ucrt/               # Universal CRT source
│
├── Musa.Runtime/                    # Overlay layer + main project
│   ├── Musa.Runtime.props           # Version & path configuration
│   ├── Musa.Runtime.vcxproj         # Aggregate library
│   ├── Musa.Runtime.CRT.vcxproj     # CRT sub-library
│   ├── Musa.Runtime.STL.vcxproj     # STL sub-library
│   ├── Musa.Runtime.VCRT.vcxproj    # VCRT sub-library
│   ├── Musa.Runtime.UCRT.vcxproj    # UCRT sub-library
│   ├── Musa.Runtime.Math.vcxproj    # Math sub-library
│   ├── Build.Microsoft.VisualC.Library.bat  # Math extraction script
│   ├── Musa.Runtime.Math.Exclusion.txt      # Objects to exclude from Math
│   ├── universal.h                  # Shared precompiled header
│   ├── universal.cpp                # Shared precompiled source
│   ├── kext/                        # Kernel-mode extensions
│   │   ├── kallocator.h             # STL-compatible kernel allocator
│   │   ├── knew.h                   # Operator new declarations
│   │   ├── kmalloc.h                # kmalloc/kfree API
│   │   ├── kmalloc.cpp              # kmalloc implementation
│   │   ├── kfree.cpp                # kfree implementation
│   │   ├── new_km.cpp               # Kernel-mode new
│   │   ├── delete_km.cpp            # Kernel-mode delete
│   │   └── thread_local.h           # Thread-local (incomplete)
│   ├── MSVC/                        # VC overlay files
│   │   └── 14.44.35207/crt/         # Overridden VC sources
│   └── UCRT/                        # UCRT overlay files
│       └── 10.0.26100.0/ucrt/       # Overridden UCRT sources
│
├── Musa.Runtime.NuGet/              # NuGet packaging
│   ├── Musa.Runtime.nuspec          # Package manifest
│   ├── Musa.Runtime.props           # NuGet build props
│   ├── Musa.Runtime.Config.props    # Consumer include/lib paths
│   └── Musa.Runtime.Config.targets  # Consumer linker deps
│
├── Musa.Runtime.TestForDriver/      # Test driver
│   ├── TestForDriver.Main.cpp       # DriverMain entry point
│   ├── Universal.h / Universal.cpp  # Test precompiled header
│   └── Musa.Tests/                 # Unit test definitions
│
├── Boost.Math/                      # Math library submodule
│   └── include/boost/math/          # Special math functions
│
├── Output/                          # Build artifacts
└── Publish/                         # Distribution output
```

## Build Process

### Step-by-Step

1. **Clean:** `BuildAllTargets.cmd` removes the `Output/` directory
2. **Environment:** Initializes Visual Studio build environment via `InitializeVisualStudioEnvironment.cmd`
3. **MSBuild:** Runs `MSBuild -binaryLogger:Output\BuildAllTargets.binlog -m BuildAllTargets.proj`
4. **Parallel Build:** The `-m` flag enables parallel compilation across sub-projects
5. **Publish:** Post-build targets copy artifacts to `Publish/`

### Sub-Project Build Order

From `Musa.Runtime.slnx`:

```
Musa.Runtime.CRT.vcxproj     ─┐
Musa.Runtime.STL.vcxproj     ─┤
Musa.Runtime.VCRT.vcxproj    ─┼──→ Musa.Runtime.vcxproj (aggregate)
Musa.Runtime.UCRT.vcxproj    ─┤
Musa.Runtime.Math.vcxproj    ─┘
```

The aggregate library (`Musa.Runtime.vcxproj`) has build dependencies on all five sub-libraries and links them together.

### Precompiled Headers

All projects except Math use a shared precompiled header:

- **Header:** `universal.h`
- **Source:** `universal.cpp` (creates the PCH)
- **Forced include:** All compilation units include `universal.h` automatically

`universal.h` sets up:
- `_NO_CRT_STDIO_INLINE` — Disable inline stdio (NLS TODO)
- `_CRT_SECURE_NO_WARNINGS` — Suppress security warnings
- `time_t` type definition (32/64-bit based on `_USE_32BIT_TIME_T`)
- `<Veil.h>` inclusion — Core system header
- Warning suppressions: 4005, 4189, 4245, 4457, 4499, 4838

### Platform Support

| Platform | Status | Notes |
|---|---|---|
| x64 | ✅ Full | Primary development target |
| ARM64 | ⚠️ Partial | Files exist, some excluded |
| Win32 | ❌ Disabled | Platform props commented out |

ARM64-specific files:
- `MARMASM` assembly files for exception handlers
- `riscchandler.cpp`, `risctrnsctrl.cpp` in VCRT
- Platform-specific exclusions via `ExcludedFromBuild` conditions

### Math Library Build

The Math library uses a unique build approach — it extracts objects from the UCRT library rather than compiling source files:

```
1. Build libucrt[d].lib (from UCRT project)
2. Run Build.Microsoft.VisualC.Library.bat:
   lib libucrt[d].lib /NOLOGO /OUT=Musa.Runtime.Math.lib
3. Remove excluded objects:
   for each obj in Musa.Runtime.Math.Exclusion.txt:
     lib Musa.Runtime.Math.lib /REMOVE:obj /IGNORE:4006,4014
```

This ensures mathematical consistency with the UCRT while allowing kernel-incompatible objects to be removed.

## Version Management

### Current Versions

| Component | Version | Location |
|---|---|---|
| VC Runtime | 14.44.35207 | `MSVC/14.44.35207/` |
| UCRT | 10.0.26100.0 | `UCRT/10.0.26100.0/` |

### Version Properties

Defined in `Musa.Runtime/Musa.Runtime.props`:

```xml
<!-- Overlay (top layer) versions -->
<Musa_Runtime_VC_Version_Overlay>14.44.35207</Musa_Runtime_VC_Version_Overlay>
<Musa_Runtime_UCRT_Version_Overlay>10.0.26100.0</Musa_Runtime_UCRT_Version_Overlay>

<!-- Base (bottom layer) versions -->
<Musa_Runtime_VC_Version>14.44.35207</Musa_Runtime_VC_Version>
<Musa_Runtime_UCRT_Version>10.0.26100.0</Musa_Runtime_UCRT_Version>
```

### Path Resolution

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

### Updating SDK Version

To update to a newer MSVC/UCRT version:

1. Place the new SDK source in `Microsoft.VisualC.Runtime/MSVC/<version>/` and `UCRT/<version>/`
2. Create overlay directories in `Musa.Runtime/MSVC/<version>/` and `UCRT/<version>/`
3. Update version properties in `Musa.Runtime.props`
4. Copy/merge overlay files from the previous version (review each for compatibility)
5. Run the build and fix any compilation issues

## Publish Pipeline

After a successful build, artifacts are published to `Publish/`:

```
Publish/
├── LICENSE                      # MIT license
├── README.md                    # Project readme
├── Include/
│   ├── kallocator.h             # Public: STL allocator
│   ├── knew.h                   # Public: operator new
│   ├── thread_local.h           # Public: thread-local (incomplete)
│   └── kmalloc.h                # Public: kmalloc API
└── Library/
    ├── {Configuration}\         # Debug or Release
    │   └── {Platform}\          # x64 or ARM64
    │       ├── Musa.Runtime.lib # Aggregate library
    │       └── Musa.Runtime.DriverEntry.obj  # DriverEntry stub
```

The `Musa.Runtime.DriverEntry.obj` is published from `Musa.Runtime.CRT.vcxproj`'s `sys_main.obj` output. This object provides the `DriverEntry` → `DriverMain` delegation.

## Build Customizations

### Mile.Project.Configurations

Musa.Runtime uses the [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows) build framework:

- `Mile.Project.Platform.x64.props` / `ARM64.props` — Platform configuration
- `Mile.Project.Cpp.Default.props` — Default C++ settings
- `Mile.Project.Cpp.props` / `Mile.Project.Cpp.targets` — C++ build logic
- `marmasm.props` / `marmasm.targets` — ARM64 assembly support

Key properties:
- `MileProjectType` = `StaticLibrary`
- `MileProjectUseKernelMode` = `true`
- `MileProjectUseWindowsDriverKit` = `true`
- `MileProjectOutputPath` = `$(MSBuildThisFileDirectory)Output/`

### Compiler Flags

Applied via `Musa.Runtime.props`:

```xml
<AdditionalOptions>/Zc:__cplusplus /Zc:threadSafeInit-</AdditionalOptions>
<ExceptionHandling>Async</ExceptionHandling>
<CallingConvention>Cdecl</CallingConvention>
<ConformanceMode>false</ConformanceMode>
<MultiProcessorCompilation>true</MultiProcessorCompilation>
<WholeProgramOptimization>false</WholeProgramOptimization>
<DebugInformationFormat>OldStyle</DebugInformationFormat>
```

### MASM Settings

x86 assembly files use SAFESEH:
```xml
<UseSafeExceptionHandlers>true</UseSafeExceptionHandlers>
```

## Build Output Artifacts

| Artifact | Source Project | Purpose |
|---|---|---|
| `Musa.Runtime.CRT.lib` | CRT.vcxproj | C runtime, new/delete, SEH, GS |
| `Musa.Runtime.STL.lib` | STL.vcxproj | Standard Template Library |
| `Musa.Runtime.VCRT.lib` | VCRT.vcxproj | VC runtime, exceptions, RTTI |
| `Musa.Runtime.UCRT.lib` | UCRT.vcxproj | Universal CRT, heap, stdlib |
| `Musa.Runtime.Math.lib` | Math.vcxproj | Math functions (extracted from UCRT) |
| `Musa.Runtime.lib` | Musa.Runtime.vcxproj | Aggregate of all above |
| `Musa.Runtime.DriverEntry.obj` | CRT.vcxproj (sys_main.obj) | DriverEntry → DriverMain delegate |
| `BuildAllTargets.binlog` | BuildAllTargets.cmd | Binary MSBuild log for diagnostics |
