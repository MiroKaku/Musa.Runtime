# Musa.Runtime Kernel-Mode CRT Handoff

## Project Overview

Musa.Runtime 将 Microsoft Visual C++ UCRT 移植到 Windows 内核模式运行。通过 overlay 机制（`Musa.Runtime/UCRT/`），用内核兼容版本替换/补充 base SDK（`Microsoft.VisualC.Runtime/UCRT/`）的源文件。

## Build

```
cd /mnt/d/DevHub/Code/Musa.Runtime
powershell -File fix.ps1             # 修复 UTF-8 BOM（git add 前必须运行）
git add -A && git commit -m "..."
BuildAllTargets.cmd                  # 全平台构建（x64/ARM64, Debug/Release）
```

**重要**: 任何文件写入后必须在 `git add` 前运行 `fix.ps1`，否则 C4819 错误。

## Architecture

### Overlay 模式
- `$(Musa_Runtime_UCRT_ToolsInstallDir)` → `Microsoft.VisualC.Runtime/UCRT/10.0.28000.0/`（base SDK）
- `$(Musa_Runtime_UCRT_ToolsInstallDir_Overlay)` → `Musa.Runtime/UCRT/10.0.28000.0/`（overlay）
- Overlay 文件同名替换 base SDK 文件。`_ctype.cpp`、`kernel_initializers.cpp` 等 overlay 在链接时覆盖 base SDK 同符号。
- 同步 26100 版本: `cp Musa.Runtime/UCRT/10.0.28000.0/ucrt/X/Y.cpp Musa.Runtime/UCRT/10.0.26100.0/ucrt/X/Y.cpp`

### CRT 初始化链
```
DriverEntry → __scrt_common_main_seh
  → __scrt_initialize_crt()
    → __acrt_initialize()
      → __acrt_initializers[] table ← overlay initialization.cpp 控制
        → initialize_pointers()
        → __acrt_initialize_winapi_thunks()  (overlay winapi_thunks.cpp)
        → initialize_global_state_isolation()
        → __acrt_initialize_locks()          (overlay locks.cpp)
        → __acrt_initialize_heap()
        → __acrt_initialize_ptd()            (overlay per_thread_data.cpp)
        → __acrt_initialize_lowio()          ← 刚刚从 _UCRT_ENCLAVE_BUILD 守卫中释放
        → __acrt_initialize_command_line()   ← kernel_initializers.cpp stub
  → _initterm_e(__xi_a, __xi_z)  ← 遍历 .CRT$XIA→.CRT$XIZ
    → __acrt_timeset_initializer  (timeset_initializer.cpp → .CRT$XIC)
    → __acrt_stdio_initializer    (kernel_initializers.cpp → __acrt_initialize_stdio)
  → DriverMain (test entry)
```

## 已修复的关键问题

### 1. tzset 崩溃 (BSOD #1)
- **原因**: `__acrt_timeset_initializer` 在 overlay 中定义为普通函数，未放入 `.CRT$XIC` 函数指针数组，`_initterm_e` 找不到它。
- **修复**: 加入 base SDK `initializers/timeset_initializer.cpp` 到 vcxproj（内含 `_CRTALLOC(".CRT$XIC") _PIFV const __acrt_timeset_initializer`），从 `kernel_initializers.cpp` 删除错误定义。

### 2. printf 递归 (BSOD #2)
- **原因**: overlay `output.cpp` 中 `_vsnprintf` → `__stdio_common_vsprintf` → `_vsnprintf` 无限递归。
- **修复**: 重命名 overlay 为 `printf_wrappers.cpp`，加入 base SDK `output.cpp`（真正的 `__stdio_common_vsprintf`）。printf 链: `snprintf`(overlay) → `_vsnprintf`(overlay) → `__stdio_common_vsprintf`(base SDK output.cpp)。
- 补充依赖: `cvt.cpp`（浮点格式化 `__acrt_fp_format`）、`cfout.cpp`、`_fptostr.cpp`。

### 3. locale NULL 指针 (BSOD #3)
- **原因**: kernel-mode `__acrt_initial_locale_data._locale_pctype` 为 NULL。
- **修复**: `_pctype_data` 改为非 static，`extern "C"` 声明。nlsdata.cpp 引用 `_pctype_data + 1` 初始化 `_locale_pctype`。

### 4. lconv NULL 指针 (BSOD #4)
- **原因**: kernel-mode `__acrt_initial_locale_data.lconv` 为 NULL，strtod 访问 `lconv->decimal_point` 崩溃。
- **修复**: 加入 base SDK `localeconv.cpp`（提供 `__acrt_lconv_c`，含 `decimal_point = "."`）。

### 5. _nhandle 未初始化 (BSOD #5)
- **原因**: `_UCRT_ENCLAVE_BUILD` 守卫排除了 `__acrt_initialize_lowio` 初始化表条目，`_nhandle` 保持 0。
- **修复**: 在 overlay `initialization.cpp` 中移除 lowio init 的 `#ifndef _UCRT_ENCLAVE_BUILD` 守卫。补充 `__acrt_initialize_command_line` stub。
- `__acrt_stdio_initializer` stub 改为调用 `__acrt_initialize_stdio()`（初始化 `_iob`/`_nstream`/`__piob`）。
- 内核模式 `ioinit.cpp` 调用 `GetStdHandle` → 失败 → `_iob._file = _NO_CONSOLE_FILENO`(-2) → `isatty.cpp:28` 正确处理 -2。

## 关键 overlay 文件及职责

| 文件 | 提供 | 依赖 |
|------|------|------|
| `internal/kernel_initializers.cpp` | `__acrt_stdio_initializer` → `__acrt_initialize_stdio`<br>`__acrt_initialize_command_line` stub<br>`__acrt_clock_initializer` stub<br>`__acrt_fmode_initializer` stub<br>`_query_app_type` → `_crt_unknown_app` | - |
| `internal/dbgstubs.cpp` | `_CrtDbgReportW` Release stub（Math lib 引用） | - |
| `convert/_ctype.cpp` | 完整 ctype 函数族 (isalpha/isalnum/iswctype 等)<br>`_pctype_data` 全局表 | 自包含 |
| `stdio/printf_wrappers.cpp` | `_vsnprintf`/`_vsnwprintf`/`snprintf`/`sscanf`/`_vscprintf`/`_vscwprintf` | base SDK `output.cpp`/`input.cpp` |
| `internal/charconv.cpp` | `__acrt_MultiByteToWideChar`/`__acrt_WideCharToMultiByte`（仅 CP_ACP/CP_UTF8）<br>`_mbtowc_internal`/`__mbsrtowcs_utf8`/`_wctomb_internal`<br>`__mblen_utf8`/`__c16rtomb_utf8`（printf 引擎依赖） | Musa.Core thunks |
| `internal/initialization.cpp` | CRT 初始化表（已移除 lowio 的 `_UCRT_ENCLAVE_BUILD` 守卫） | base SDK |
| `locale/nlsdata.cpp` | 内核模式 locale 数据（`_pctype_data+1`、`&__acrt_lconv_c`、`__lc_time_c`） | `_ctype.cpp`、`localeconv.cpp` |

## 新增 base SDK 文件（vcxproj）

为支持完整 printf 浮点格式化和 locale:
- `convert/cvt.cpp` — `__acrt_fp_format`
- `convert/cfout.cpp` — `__acrt_fltout`
- `convert/_fptostr.cpp` — `__acrt_fp_strflt_to_string`
- `locale/localeconv.cpp` — `__acrt_lconv_c`
- `stdio/output.cpp` — `__stdio_common_vsprintf`（printf 引擎）
- `initializers/timeset_initializer.cpp` — `.CRT$XIC` 时区初始化器

## 已知未处理

- **invalid_parameter handler**: 内核模式下 `_invoke_watson` → `__fastfail` 导致 BSOD。用户认为已有内核版 `_invoke_watson`，但当前 base SDK 版本仍调用 `__fastfail`。`_isatty(-1)` 测试已删除，`_isatty(_fileno(stdout))` 返回 -2 走 early-return 路径避免了问题。
- **_CrtDbgReportW**: Math lib（`contrlfp.obj`/`fpexcept.obj`）Release 模式下仍引用，由 `dbgstubs.cpp` 提供空 stub。
- **其余 8 个 initializer 文件**: `clock`/`console_input`/`console_output`/`fmode`/`locale`/`multibyte`/`stdio`/`tmpfile` 的 initializer 未加入 vcxproj，但目前未触发问题。

## 测试

- 测试文件: `Musa.Runtime.TestForDriver/TestForDriver.Main.cpp`
- 运行环境: Hyper-V 虚拟机 + WinDbg 内核调试
- WinDbg MCP 可用（`mcp__windbg_*` 工具）

## 调试记录

每次构建后重启 VM:
```
mcp__windbg_execute_command(command: ".reboot")
```
等待 VM 重连后 `mcp__windbg_get_execution_state` → break in → `!analyze -v` 分析崩溃。
