# Musa.Runtime Overlay 需补充的符号

> 由 UCRT 解锁计划触发 · 2026-05-04

## 概述

向 `Musa.Runtime.UCRT.vcxproj` 添加 112 个基础 SDK 源文件后，链接 TestForDriver 时出现 35 个未解析符号。这些符号分为两类：

1. **kernel32 API 缺失 thunk** → 需在 Musa.Core 中补充（见 `Musa.Core/docs/missing-apis.md`）
2. **UCRT 内部/CRT 初始化器** → 需在 Musa.Runtime overlay 中补充（本文档）

---

## 已由 overlay 提供的符号（无需处理）

| 符号 | 提供位置 |
|---|---|
| `__lc_time_c` | `locale/nlsdata.cpp:32` |
| `__lc_wcstolc` | `locale/wsetlocale.cpp:1338` |
| `__acrt_initialize_multibyte` | `internal/initialization.cpp:278` — 已有 `#if !defined NTOS_KERNEL_RUNTIME` 守卫 |

---

## 需补充的 overlay 实现

### 1. CRT 初始化器（4 个）

| 符号 | 引用来源 | 当前状态 | 实现方案 |
|---|---|---|---|
| `__acrt_stdio_initializer` | overlay `stdio/_file.cpp:45` via `_CRT_LINKER_FORCE_INCLUDE` | overlay 已引用但无实现体 | 已有声明，需在 overlay 中提供空函数体 |
| `__acrt_stdio_terminator` | overlay `stdio/_file.cpp:46` | 同上 | 同上 |
| `__acrt_clock_initializer` | 基础 SDK `time/clock.cpp` | 无 | 内核模式可空实现 |
| `__acrt_timeset_initializer` | 基础 SDK `time/timeset.cpp` | 无 | 内核模式可空实现 |

**实现（添加到 overlay `stdio/_file.cpp` 末尾）：**
```c
// 新增
extern "C" void __cdecl __acrt_stdio_initialize()
{
    // 内核模式：stdio 无全局状态需初始化
}

extern "C" void __cdecl __acrt_stdio_terminate()
{
    // 内核模式：stdio 无全局状态需清理
}
```

**实现（添加到 overlay `internal/initialization.cpp`）：**
```c
extern "C" void __cdecl __acrt_clock_initializer()
{
    // 内核模式：无需用户态时钟初始化
}

extern "C" void __cdecl __acrt_timeset_initializer()
{
    // 内核模式：无需用户态时区初始化
}
```

### 2. 字符转换（5 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `__acrt_MultiByteToWideChar` | `time/strftime.cpp`, `env/putenv.cpp`, `env/environment_initialization.cpp`, `stdio/read.cpp`, overlay `locale/wsetlocale.cpp` | overlay `convert/` 中提供，调用 `RtlMultiByteToUnicodeN` |
| `__acrt_WideCharToMultiByte` | `env/get_environment_from_os.cpp`, `stdio/write.cpp`, `time/strftime.cpp`, `env/putenv.cpp`, `time/tzset.cpp`, `env/environment_initialization.cpp` | overlay `convert/` 中提供，调用 `RtlUnicodeToMultiByteN` |
| `_mbtowc_internal` | 基础 SDK `stdio/write.cpp` | overlay `convert/` 中提供 |
| `__crt_mbstring::__mbsrtowcs_utf8` | 基础 SDK `stdio/write.cpp` | overlay `convert/` 中提供 |
| `_putwch_nolock` | 基础 SDK `stdio/write.cpp`（控制台宽字符输出） | 内核模式空实现 |

**`__acrt_MultiByteToWideChar` 实现（添加到 overlay `convert/`）：**
```c
extern "C" int __cdecl __acrt_MultiByteToWideChar(
    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr,
    int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    // 内核模式使用 RtlMultiByteToUnicodeN 或 RtlUTF8ToUnicodeN
    // 简化实现：仅支持 UTF-8 / ACP
    NTSTATUS Status;
    ULONG cbSrc = (cbMultiByte == -1) ? (ULONG)strlen(lpMultiByteStr) + 1 : (ULONG)cbMultiByte;
    ULONG cchDst = cchWideChar;
    
    Status = RtlMultiByteToUnicodeN(lpWideCharStr, cchDst * sizeof(WCHAR),
        NULL, lpMultiByteStr, cbSrc);
    
    if (!NT_SUCCESS(Status)) return 0;
    return (int)(NT_SUCCESS(Status) ? cbSrc : 0);
}
```

### 3. 多字节字符串（4 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `_ismbblead` | 基础 SDK `filesystem/splitpath.cpp` | overlay `mbstring/` 中提供 |
| `_mbsdec` | 基础 SDK `filesystem/makepath.cpp` | overlay `mbstring/` 中提供 |
| `_wcsnicoll` | 基础 SDK `env/getenv.cpp`, `env/setenv.cpp` | overlay `string/` 中提供 |
| `_strnicoll` | 基础 SDK `env/getenv.cpp`, `env/setenv.cpp` | overlay `string/` 中提供 |

**实现（添加到 overlay `mbstring/` 或 `internal/`）：**
```c
// 内核模式：仅支持 ASCII / UTF-8，不实现完整多字节
extern "C" int __cdecl _ismbblead(unsigned int c)
{
    // 内核模式无多字节字符集支持
    UNREFERENCED_PARAMETER(c);
    return 0;
}

extern "C" unsigned char* __cdecl _mbsdec(const unsigned char* start, const unsigned char* current)
{
    // 向后遍历一个多字节字符 — 内核模式仅 ASCII
    if (current > start) return (unsigned char*)(current - 1);
    return (unsigned char*)start;
}

// 字符串排序：内核模式使用简单的不区分大小写比较
extern "C" int __cdecl _strnicoll(const char* s1, const char* s2, size_t n)
{
    return _strnicmp(s1, s2, n);
}

extern "C" int __cdecl _wcsnicoll(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    return _wcsnicmp(s1, s2, n);
}
```

### 4. UCRT 内部 — 委托给 kernel32（5 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `__acrt_GetSystemTimePreciseAsFileTime` | `time/time.cpp` | 委托给 `GetSystemTimeAsFileTime` thunk（需 Musa.Core 先实现） |
| `__acrt_SetEnvironmentVariableA` | `env/setenv.cpp` | 将 ANSI 参数转为宽字符，调用 `SetEnvironmentVariableW` thunk |
| `__acrt_AreFileApisANSI` | `time/tzset.cpp` | 内核模式始终返回 FALSE |
| `__acrt_GetDateFormatEx` | `time/wcsftime.cpp` | 委托给 `GetDateFormatEx` thunk（P3，非阻塞） |
| `__acrt_GetTimeFormatEx` | `time/wcsftime.cpp` | 委托给 `GetTimeFormatEx` thunk（P3，非阻塞） |

**实现（添加到 overlay `internal/winapi_thunks.cpp`）：**
```c
extern "C" void __cdecl __acrt_GetSystemTimePreciseAsFileTime(LPFILETIME lpTime)
{
    // 委托：内核模式使用 KeQuerySystemTimePrecise 实现 GetSystemTimeAsFileTime
    LARGE_INTEGER Time;
    KeQuerySystemTimePrecise(&Time);
    lpTime->dwLowDateTime  = Time.LowPart;
    lpTime->dwHighDateTime = Time.HighPart;
}

extern "C" int __cdecl __acrt_AreFileApisANSI()
{
    return FALSE;  // 内核模式无 ANSI/OEM 概念
}

extern "C" int __cdecl __acrt_SetEnvironmentVariableA(LPCSTR lpName, LPCSTR lpValue)
{
    // 内核模式不支持，返回 FALSE
    RtlSetLastWin32Error(ERROR_NOT_SUPPORTED);
    return 0;
}
```

### 5. Locale 数据（1 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `___lc_codepage_func` | 基础 SDK `time/tzset.cpp` | overlay `locale/nlsdata.cpp` 中已有 `__lc_time_c`，需额外提供 |

**实现（添加到 overlay `locale/nlsdata.cpp`）：**
```c
extern "C" unsigned int __cdecl ___lc_codepage_func()
{
    // 内核模式：使用 UTF-8 代码页
    return CP_UTF8;
}
```

### 6. 应用类型（1 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `_query_app_type` | 基础 SDK `lowio/osfinfo.cpp` | overlay `misc/` 或 `internal/` 中提供 |

```c
// 来自 appmodel.h，内核模式无此概念
extern "C" unsigned long __cdecl _query_app_type()
{
    return 0;  // 无应用类型
}
```

### 7. 数字转换（1 个）

| 符号 | 引用来源 | 实现方案 |
|---|---|---|
| `wcstol` | 基础 SDK `time/tzset.cpp` | 内核模式使用简单实现或委托给 `wcstol` 的 C 标准实现 |

**说明：** `wcstol` 是标准 C 函数，应在基础 SDK `stdlib/wcstol.cpp` 中。当前 overlay 未编译此文件。解决方式：将 `stdlib/wcstol.cpp` 加入 UCRT vcxproj 编译（该文件不依赖外部 API，可直接编译）。

---

## 汇总

| 类别 | 符号数 | 需新增源文件 |
|---|---|---|
| CRT 初始化器 | 4 | 修改 `stdio/_file.cpp`、`internal/initialization.cpp` |
| 字符转换 | 5 | `convert/` 或新文件 |
| 多字节/字符串 | 4 | `mbstring/` 或 `internal/` |
| UCRT→kernel32 委托 | 5 | 修改 `internal/winapi_thunks.cpp` |
| Locale | 1 | 修改 `locale/nlsdata.cpp` |
| 应用类型 | 1 | `misc/` 或 `internal/` |
| 数字转换 | 1 | 将 `stdlib/wcstol.cpp` 加入编译 |
| **总计** | **21** | |

## 优先级建议

- **P0**：CRT 初始化器（4 个）— overlay 已引用但缺实现，必须补充
- **P1**：委托类（5 个）— 需配合 Musa.Core 的 P1 thunk 实现
- **P2**：字符转换（5 个）+ 多字节（4 个）+ locale（1 个）+ 应用类型（1 个）
- **P3**：数字转换（1 个）— 简单加入编译即可
