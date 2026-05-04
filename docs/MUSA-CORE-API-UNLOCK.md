# Musa.Runtime UCRT 目录解锁计划

> 基于 Musa.Core API 补充实现 · 2026-05-04

## 背景

Musa.Runtime 通过 `Musa.Runtime.Math.Exclusion.txt` 排除了 17 个 UCRT 目录的编译。这些目录依赖的 Win32 kernel32 API 在 Musa.Core 中尚未实现，导致无法参与编译。

本次在 Musa.Core 中补充了 40+ 个 kernel32 API thunk，解锁了绝大多数目录。

---

## 第二轮补充（2026-05-04）· 9 个 API

### 时间 API

| API | 实现文件 | UCRT 解锁 |
|---|---|---|
| `GetSystemTimeAsFileTime` | `KernelBase.RealTime.cpp` | `time/time.cpp` (`timespec_get` 精度提升) |

### 环境变量（完整支持）

| API | 实现文件 | UCRT 解锁 |
|---|---|---|
| `SetEnvironmentVariableW` | `KernelBase.System.cpp` | `env/setenv.cpp` (`_putenv`, `_wputenv` 写入) |
| `GetEnvironmentStringsW` | `KernelBase.System.cpp` | `env/get_environment_from_os.cpp` |
| `FreeEnvironmentStringsW` | `KernelBase.System.cpp` | 同上 |

> 实现细节：`SetEnvironmentVariableW` 仅写入 KPEB 进程内链表（shared/exclusive PEB lock），不修改系统注册表。`GetEnvironmentVariableW` 先查 KPEB 再 fallback 注册表。`GetEnvironmentStringsW` 优先 KPEB，非空走注册表。

### 控制台 I/O

| API | 实现文件 | 说明 |
|---|---|---|
| `GetConsoleOutputCP` | `KernelBase.Console.cpp` | 返回 ANSI 代码页 |
| `GetConsoleMode` | `KernelBase.Console.cpp` | stub，返回不支持 |
| `ReadConsoleW` | `KernelBase.Console.cpp` | stub，返回不支持 |

> 内核驱动无控制台句柄。stub 返回 FALSE + `ERROR_NOT_SUPPORTED`，UCRT 会自动回退到按普通文件句柄处理。

### 日期/时间格式化

| API | 实现文件 | UCRT 解锁 |
|---|---|---|
| `GetDateFormatEx` | `KernelBase.NLS.cpp` | `time/wcsftime.cpp` |
| `GetTimeFormatEx` | `KernelBase.NLS.cpp` | 同上 |

> ISO 8601 回退路径（`YYYY-MM-DD` / `HH:MM:SS`），与 kernel32 原生行为一致。

---

## 第一轮补充（2026-05-03）· 30+ API

### `lowio/` — 18 文件
POSIX 风格低层 I/O（`_open`, `_read`, `_write`, `_close`, `_lseek` 等）

| API | 实现文件 |
|---|---|
| `GetFileType` | `KernelBase.File.cpp` |
| `ReadFile` | `KernelBase.File.cpp` |
| `WriteFile` | `KernelBase.File.cpp` |
| `FlushFileBuffers` | `KernelBase.File.cpp` |
| `SetFilePointer` / `SetFilePointerEx` | `KernelBase.File.cpp` |
| `CreateFileW` | `KernelBase.File.cpp` |

### `stdio/` — 44 文件
标准 I/O（`fopen`, `fread`, `fwrite`, `fprintf`, `fscanf` 等）

| API | 实现文件 |
|---|---|
| 所有 lowio API | `KernelBase.File.cpp` |
| `GetTempPathW` | `KernelBase.File.cpp` |

### `filesystem/` — 17 文件
文件系统操作（`_stat`, `_access`, `_unlink`, `_mkdir`, `_rmdir`, `_rename` 等）

| API | 实现文件 |
|---|---|
| `GetFileAttributesExW` | `KernelBase.File.cpp` |
| `SetFileAttributesW` | `KernelBase.File.cpp` |
| `DeleteFileW` | `KernelBase.File.cpp` |
| `CreateDirectoryW` | `KernelBase.File.cpp` |
| `RemoveDirectoryW` | `KernelBase.File.cpp` |
| `MoveFileExW` | `KernelBase.File.cpp` |
| `FindFirstFileExW` / `FindNextFileW` / `FindClose` | `KernelBase.File.cpp` |
| `GetDriveTypeW` | `KernelBase.File.cpp` |

### `time/` — 18 文件
时间函数（`asctime`, `localtime`, `strftime`, `mktime` 等）

| API | 实现文件 |
|---|---|
| `GetSystemTime` | `KernelBase.System.Times.cpp` |
| `GetLocalTime` | `KernelBase.System.Times.cpp` |
| `FileTimeToSystemTime` | `KernelBase.System.Times.cpp` |
| `SystemTimeToFileTime` | `KernelBase.System.Times.cpp` |
| `FileTimeToLocalFileTime` | `KernelBase.System.Times.cpp` |
| `LocalFileTimeToFileTime` | `KernelBase.System.Times.cpp` |
| `GetTimeZoneInformation` | `KernelBase.System.Times.cpp` |
| `GetDynamicTimeZoneInformation` | `KernelBase.System.Times.cpp` |

### `env/` — 6 文件
环境变量（`getenv`, `putenv`, `searchenv` 等）

| API | 实现文件 |
|---|---|
| `GetEnvironmentVariableW` | `KernelBase.System.cpp` |
| `ExpandEnvironmentStringsW` | `KernelBase.System.cpp` |

### `misc/` — 2 文件
`getcwd` / `chdir`

| API | 实现文件 |
|---|---|
| `GetCurrentDirectoryW` | `KernelBase.System.cpp` → `Ntdll.Path.cpp` |
| `SetCurrentDirectoryW` | `KernelBase.System.cpp` → `Ntdll.Path.cpp` |

---

## NTDLL 兼容层新增

| API | 实现文件 | 说明 |
|---|---|---|
| `RtlDosPathNameToNtPathName_U` | `Ntdll.Path.cpp` | BOOLEAN 返回，DOS→NT 路径转换 |
| `RtlDosPathNameToNtPathName_U_WithStatus` | `Ntdll.Path.cpp` | NTSTATUS 返回变体 |
| `RtlGetCurrentDirectory_U` | `Ntdll.Path.cpp` | 基于 KPEB 的当前目录 |
| `RtlSetCurrentDirectory_U` | `Ntdll.Path.cpp` | 基于 KPEB 的当前目录设置 |

---

## 部分解锁

### `startup/` — 1 文件
| `GetCommandLineW` | `KernelBase.System.cpp` | 返回空字符串（内核无命令行） |

### `internal/` — 1 文件
| `LoadLibraryExA` | `KernelBase.LibraryLoader.cpp` | stub，返回 `STATUS_NOT_SUPPORTED` |

---

## 仍排除（2 目录，9 文件）

### `exec/` — 8 文件
进程创建（`spawn`, `exec`, `system` 等）

阻塞于 `CreateProcessW`。内核驱动极少需要创建用户进程。完整实现需要 section 创建、进程对象创建、线程创建、环境块传递、PEB 初始化等，复杂度极高。

### `stdlib/system()` — 1 文件
阻塞于 `CreateProcessW`。

---

## 有意排除

| 目录 | 原因 |
|---|---|
| `mbstring/` | 日文 JIS/Shift-JIS 字符处理 |
| `conio/` | 控制台 I/O（`cprintf`/`getch`） |
| `desktopcrt/` | 桌面专用 CRT |
| `crtw32/` | Win32 特定 CRT |

---

## 已由 overlay 编译（不依赖 Exclusion.txt）

| 目录 | 说明 |
|---|---|
| `heap/` | `kmalloc`/`kfree` 替代 |
| `convert/` | `mbstowcs`/`wcstombs` |
| `locale/` | `wsetlocale`/`nlsdata` |
| `stdlib/` | `abs`/`qsort`/`bsearch` 等纯计算函数 |
| `string/` | 纯内存操作 |
| `initializers/` | 初始化器 |
| `internal/`（部分）| `initialization`/`locks`/`per_thread_data` |
| `misc/`（部分）| `dbgrptt`/`signal` |
| `startup/`（thread）| 线程启动 |
| `stdio/`（output）| inline 辅助函数 |

---

## 实施统计

| 指标 | 第一轮 | 第二轮 | 合计 |
|---|---|---|---|
| 新增 API thunk | 30+ | 9 | 40+ |
| 新增源文件 | 3 | 1 (KernelBase.Console.cpp) | 4 |
| 新增头文件 | 2 | 1 (Internal/KernelBase.Console.h) | 3 |
| 修改文件 | 5 | 8 | 13 |
| 单元测试用例 | 70+ | 20+ | 90+ |
| 解锁 UCRT 文件 | 103 | — | 103 |
| 构建状态 | 0 错误 | 0 错误 | 0 错误 |
