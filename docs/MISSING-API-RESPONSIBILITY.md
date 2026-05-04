# 缺失 API 归责报告

> 由 Musa.Runtime UCRT 解锁 · 文件 I/O 测试 · 2026-05-04

## Musa.Core 职责（kernel32 API thunk）

以下 `__imp_` 前缀符号来自 `kernel32.dll`，需在 Musa.Core 中实现 thunk：

| 符号 | 来源 | 使用场景 | 已有 thunk？ |
|---|---|---|---|
| `SetEndOfFile` | `chsize.obj`（lowio） | `_chsize` 修改文件大小时调用 | 否 |
| `GetFileSizeEx` | `_flsbuf.obj`（stdio） | 刷新缓冲区时检测文件末尾 | 否 |
| `GetFileInformationByHandle` | `stat.obj`（filesystem） | `_stat` 获取文件元数据 | 否 |
| `PeekNamedPipe` | `stat.obj`（filesystem） | `_stat` 检测管道类型 | 否 |
| `SystemTimeToTzSpecificLocalTime` | `stat.obj`（filesystem） | `_stat` 转换文件时间为本地时区 | 否 |

### 建议实现

- `SetEndOfFile` — 调用 `ZwSetInformationFile(FileEndOfFileInformation)`
- `GetFileSizeEx` — 调用 `ZwQueryInformationFile(FileStandardInformation)`
- `GetFileInformationByHandle` — 调用 `ZwQueryInformationFile(FileBasicInformation)` + `ZwQueryInformationFile(FileStandardInformation)`
- `PeekNamedPipe` — stub 返回 `TRUE` + `available=0`（内核驱动不连接管道）
- `SystemTimeToTzSpecificLocalTime` — `LocalFileTimeToFileTime` 已有 thunk，可基于此实现

---

## Musa.Runtime 职责（overlay 需补充）

| 符号 | 来源 | 解决方案 |
|---|---|---|
| `__GSHandlerCheck_SEH_noexcept` | `open.obj`, `fopen.obj` | ✅ 已实现于 `MSVC/crt/vcruntime/chandler_noexcept.cpp` |
| `__acrt_fmode_initializer` | `txtmode.obj`（lowio） | ✅ 已实现于 `UCRT/internal/kernel_initializers.cpp` |
| `wcspbrk` | `stat.obj`（filesystem） | 基础 SDK 有 `string/wcspbrk.cpp`，加入 UCRT vcxproj 编译 |
| `_getdrive` | `stat.obj`（filesystem） | 基础 SDK `lowio/` 中有实现，加入 UCRT vcxproj 编译 |
| `_getcwd` / `_wgetcwd` | `fullpath.obj`（filesystem） | 基础 SDK `misc/getcwd.cpp` 提供，但 overlay 已有 `getcwd`/`chdir` 实现（依赖 `GetCurrentDirectoryW` thunk）。需检查符号冲突 |

---

## 当前测试状态

| 测试组 | 状态 | 阻塞原因 |
|---|---|---|
| lowio（open/write/read/lseek/close/tell/eof/filelength） | ✅ 通过 | — |
| stdio（fopen/fwrite/fflush/rewind/fread/fclose） | ✅ 通过 | — |
| 格式化 I/O（sprintf+fwrite/fread+sscanf） | ✅ 通过 | — |
| filesystem（mkdir/rmdir/rename/access） | ✅ 通过 | — |
| filesystem（_stat/_fullpath） | ❌ 移除 | 需 Musa.Core + Musa.Runtime 补充上述符号 |
