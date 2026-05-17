# Locale Overlay 全量精简计划

## 目标
将 UCRT 中所有涉及 locale 的源文件加入编译，创建 kernel-mode overlay 版本。保留函数签名，精简实现体。内核模式下 locale 固定为 C locale + CP_UTF8，消除所有动态 locale 分支开销。

## 原则

1. **保留函数签名** — 不改变任何函数参数
2. **消除 locale 分支** — 固定返回 C locale / CP_UTF8 结果
3. **同步 26100/28000** — 每个 overlay 文件同步到两个版本目录
4. **更新 vcxproj** — 将新文件加入编译列表

## 当前已编译的 locale 相关文件（已处理）

| 文件 | 状态 |
|---|---|
| `convert/strtod.cpp` | ✅ 已处理 |
| `convert/mbstowcs.cpp` | ✅ 已处理 |
| `convert/wcstombs.cpp` | ✅ 已处理 |
| `convert/cvt.cpp` | ✅ 已处理 |
| `convert/gcvt.cpp` | ✅ 已处理 |
| `convert/_ctype.cpp` | ✅ 已有 |
| `locale/ctype.cpp` | ✅ 已有 |
| `locale/locale_update.cpp` | ✅ 已有 |
| `locale/nlsdata.cpp` | ✅ 已有 |
| `locale/wsetlocale.cpp` | ✅ 已有 |

## 需要新增编译的 locale 相关文件

### Phase 1: convert/ (字符串转换 - _l 变体)

| # | 文件 | 涉及函数 | 内核模式行为 |
|---|---|---|---|
| 1 | `atof.cpp` | `_wtof_l`, `_atoldbl_l` | 直接调用 strtod |
| 2 | `atoldbl.cpp` | `_atoldbl_l`, `atoldbl` | 同上 |
| 3 | `c16rtomb.cpp` | `c16rtomb`, `_c16rtomb_l` | 始终 UTF-8 |
| 4 | `c32rtomb.cpp` | `c32rtomb`, `_c32rtomb_l` | 始终 UTF-8 |
| 5 | `fcvt.cpp` | `_fcvt_l`, `_gcvt_l` | `decimal_point` = `'.'` |
| 6 | `mblen.cpp` | `mblen`, `_mblen_l` | 始终 UTF-8 |
| 7 | `mbrtoc16.cpp` | `mbrtoc16`, `_mbrtoc16_l` | 始终 UTF-8 |
| 8 | `mbrtoc32.cpp` | `mbrtoc32`, `_mbrtoc32_l` | 始终 UTF-8 |
| 9 | `mbrtowc.cpp` | `mbrtowc`, `_mbrtowc_l`, `mbsrtowcs` | 始终 UTF-8 |
| 10 | `mbtowc.cpp` | `mbtowc`, `_mbtowc_l` | 始终 UTF-8 |
| 11 | `wcrtomb.cpp` | `wcrtomb`, `wcsrtombs` | 始终 UTF-8 |
| 12 | `wctomb.cpp` | `wctomb`, `_wctomb_l` | 始终 UTF-8 |
| 13 | `isctype.cpp` | `_isctype_l` | 静态 `_pctype_data` |
| 14 | `iswctype.cpp` | `iswctype`, `_iswctype_l` | 静态表 |
| 15 | `tolower_toupper.cpp` | `tolower`, `toupper`, `_tolower_l`, `_toupper_l` | ASCII 映射 |
| 16 | `towlower.cpp` | `towlower`, `_towlower_l` | ASCII 映射 |
| 17 | `towupper.cpp` | `towupper`, `_towupper_l` | ASCII 映射 |
| 18 | `wctrans.cpp` | `wctrans`, `towctrans` | 返回 NULL / 原样 |
| 19 | `wctype.cpp` | `wctype`, `_wctype_l` | 静态表 |

### Phase 2: locale/ (locale 管理)

| # | 文件 | 涉及函数 | 内核模式行为 |
|---|---|---|---|
| 20 | `localeconv.cpp` | `localeconv` | 返回 `&__acrt_lconv_c` (已有) |
| 21 | `setlocale.cpp` | `setlocale` | 返回 `nullptr` + `ENOTSUP` |
| 22 | `glstatus.cpp` | `__globallocalestatus` | 静态变量 |
| 23 | `locale_refcounting.cpp` | 引用计数管理 | 空操作 |

### Phase 3: string/ (字符串比较/转换)

| # | 文件 | 涉及函数 | 内核模式行为 |
|---|---|---|---|
| 24 | `strcoll.cpp` | `strcoll`, `_strcoll_l` | 等价 `strcmp` |
| 25 | `stricoll.cpp` | `stricoll`, `_stricoll_l` | 等价 `stricmp` |
| 26 | `strncoll.cpp` | `strncoll`, `_strncoll_l` | 等价 `strncmp` |
| 27 | `strxfrm.cpp` | `strxfrm`, `_strxfrm_l` | 等价 `strcpy` + 返回长度 |
| 28 | `strnicmp.cpp` | `strnicmp`, `_strnicmp_l` | ASCII 比较 |
| 29 | `wcscoll.cpp` | `wcscoll`, `_wcscoll_l` | 等价 `wcscmp` |
| 30 | `wcsicoll.cpp` | `wcsicoll`, `_wcsicoll_l` | 等价 `wcsicmp` |
| 31 | `wcsncoll.cpp` | `wcsncoll`, `_wcsncoll_l` | 等价 `wcsncmp` |
| 32 | `wcsxfrm.cpp` | `wcsxfrm`, `_wcsxfrm_l` | 等价 `wcscpy` + 返回长度 |
| 33 | `wcsnicmp.cpp` | `wcsnicmp`, `_wcsnicmp_l` | ASCII 比较 |
| 34 | `strlwr.cpp` | `strlwr`, `_strlwr_l` | ASCII 小写 |
| 35 | `strupr.cpp` | `strupr`, `_strupr_l` | ASCII 大写 |
| 36 | `wcslwr.cpp` | `wcslwr`, `_wcslwr_l` | ASCII 小写 |
| 37 | `wcsupr.cpp` | `wcsupr`, `_wcsupr_l` | ASCII 大写 |
| 38 | `strdup.cpp` | `_strdup` | 标准实现 |
| 39 | `wcsdup.cpp` | `_wcsdup` | 标准实现 |
| 40 | `strnset.c` | `_strnset` | 标准实现 |
| 41 | `strnset_s.cpp` | `_strnset_s` | 标准实现 |
| 42 | `strset.c` | `_strset` | 标准实现 |
| 43 | `strset_s.cpp` | `_strset_s` | 标准实现 |
| 44 | `wcsnset.cpp` | `_wcsnset` | 标准实现 |
| 45 | `wcsnset_s.cpp` | `_wcsnset_s` | 标准实现 |
| 46 | `wcsset.cpp` | `_wcsset` | 标准实现 |
| 47 | `wcsset_s.cpp` | `_wcsset_s` | 标准实现 |

### Phase 4: mbstring/ (多字节字符串 - 87 文件)

全部 mbstring 函数都依赖 locale（`_mbscoll`, `_mbsicmp` 等）。内核模式下：
- 所有 `_l` 变体忽略 locale 参数
- 所有 collation 函数降级为简单字节比较
- 所有字符分类函数使用静态表

### Phase 5: time/ (时间格式化)

| # | 文件 | 涉及函数 | 内核模式行为 |
|---|---|---|---|
| 48 | `strftime.cpp` | `strftime` | 使用静态 `__lc_time_c` (已有) |
| 49 | `wcsftime.cpp` | `wcsftime` | 使用静态 `__lc_time_c` (已有) |

## 验证流程

对每个文件：
1. 读取 base SDK 原文件
2. 确认 locale 依赖点
3. 设计 overlay 精简方案
4. 创建 overlay 文件
5. 同步到 26100 版本
6. 添加到 vcxproj
