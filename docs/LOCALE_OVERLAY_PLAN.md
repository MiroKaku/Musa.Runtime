# Locale Overlay 精简计划

## 目标
将所有涉及 locale 的 base SDK 函数创建 overlay 版本，保留函数签名，精简实现体。内核模式下 locale 固定为 C locale + CP_UTF8，消除所有动态 locale 分支开销。

## 原则

1. **保留函数签名** — 不改变任何函数参数，避免调用方同步
2. **消除 locale 分支** — `_LocaleUpdate` / `__crt_cached_ptd_host::get_locale()` / `locale->locinfo` 相关代码直接替换为固定值
3. **每个文件逐一验证** — 先读 base SDK 原文件，确认需要 overlay 的函数，再创建 overlay
4. **同步 26100 版本** — 每个 overlay 文件同时复制到 10.0.26100.0 目录
5. **只覆盖 vcxproj 实际编译的文件** — 避免创建无用 overlay

## 验证结果 (2026-05-05)

经过逐一读取 vcxproj 中所有编译的源文件，确认以下结论：

### 需要 overlay 的文件（已创建）

| # | 文件 | 验证状态 | 精简内容 |
|---|---|---|---|
| 1 | `convert/strtod.cpp` | ✅ 已创建 | `_LocaleUpdate` → `__crt_cached_ptd_host` 固定初始 locale |
| 2 | `convert/mbstowcs.cpp` | ✅ 已创建 | `ptd.get_locale()` → 直接走 CP_UTF8 路径 |
| 3 | `convert/wcstombs.cpp` | ✅ 已创建 | `ptd.get_locale()` → 直接走 CP_UTF8 路径 |
| 4 | `convert/_ctype.cpp` | ✅ 已有 | locale 参数忽略 |

### 不需要 overlay 的文件

| 文件 | 原因 |
|---|---|
| `convert/strtox.cpp` | 无 locale 分支，纯解析逻辑 |
| `convert/cfout.cpp` | 无 locale 分支 |
| `convert/_fptostr.cpp` | `__acrt_fp_format` 不依赖 locale |
| `convert/cvt.cpp` | `__acrt_fltout` 不依赖 locale |
| `lowio/dup.cpp` | 仅用 `ptd` 做 errno 映射，无 locale 分支 |
| `lowio/dup2.cpp` | 仅用 `ptd` 做 errno 映射，无 locale 分支 |
| `lowio/lseek.cpp` | 仅用 `ptd` 做 errno 映射，无 locale 分支 |
| `lowio/close.cpp` | 仅用 `ptd` 做 errno 映射，无 locale 分支 |
| `lowio/chsize.cpp` | 仅用 `ptd` 做 errno 映射，无 locale 分支 |
| `lowio/write.cpp` | `write_requires_double_translation_nolock` 在 C locale 下永远返回 false，双转换路径不会执行 |

### 当前已存在的 overlay

| 文件 | 状态 |
|---|---|
| `locale/ctype.cpp` | ✅ 已有，locale 参数忽略 |
| `locale/locale_update.cpp` | ✅ 已有，4 个空 stub |
| `locale/nlsdata.cpp` | ✅ 已有，静态 C locale 数据 |
| `locale/wsetlocale.cpp` | ✅ 已有，大部分守卫排除 |
| `internal/charconv.cpp` | ✅ 已有，`UNREFERENCED_PARAMETER(ptd)` |
| `internal/kernel_initializers.cpp` | ✅ 已有，locale 相关 stub |
| `internal/initialization.cpp` | ✅ 已有，init 表守卫 |

## 预期效果

- 消除 `_LocaleUpdate` 构造/析构开销（strtod 等）
- 消除 `ptd.get_locale()->locinfo->...` 间接访问（mbstowcs/wcstombs）
- 内核模式 locale 始终返回 `&__acrt_initial_locale_pointers`
- `__acrt_locale_changed()` 永远为 FALSE，`__crt_cached_ptd_host` 构造函数走快速路径

## 注意事项

- 不创建 vcxproj 未引用文件的 overlay（如 mblen.cpp, mbtowc.cpp 等），避免维护负担
- write.cpp 的 locale 分支在 C locale 下自动跳过，不需要 overlay
- 所有 lowio 文件（dup/dup2/lseek/close/chsize）仅用 ptd 做 errno 映射，无 locale 依赖
