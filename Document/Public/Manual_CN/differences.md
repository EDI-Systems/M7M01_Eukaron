# M7M01 手册与内核一致性检查

检查日期：2026-11-05
检查对象：`Document/Public/Manual_CN` 中的 R4T1 中文手册 LaTeX 源文件
对照对象：`Include/`、`Source/` 中的 M7M01 内核头文件和实现
检查性质：**只读审计**。本报告不修改内核、头文件或手册正文。

## 0. 结论摘要

手册的章节结构、图表、脚注和绝大多数接口参数已经完整转换，但手册描述的是一代较旧的接口：
安全监视器内核调用、调试内核调用编号、若干宏名称和移植函数签名与当前内核不一致。
其中安全监视器 `ETHD_*` 调用在当前内核中完全不存在，属于高优先级不一致。

共确认 7 组不一致：

1. 安全监视器内核调用（`ECLV_*` / `ETHD_*`）的名称、编号和功能范围不一致。
2. 调试内核调用编号缺少 `RME_KFN_DEBUG_EXC_GET`，后续编号整体错位。
3. 返回码 `RME_ERR_CPT_KOTBL` 在当前内核中名为 `RME_ERR_CPT_KOT`。
4. 线程、页表、RVM 配置的若干宏名称不一致。
5. 服务号在手册表格中使用旧别名（`RME_INV_CRT` 等），内核中为 `RME_SVC_*`。
6. 移植接口 `_RME_Kotbl_Init`、`_RME_Thd_Fatal`、`RME_Print_*` 的名称或签名不一致。
7. 当前内核中的若干返回码在手册中没有说明。

> 2026-11-05 后续修复：第 3、4、5、6 组已修复，详见第 11 节；第 1、2、7 组仍未修复。

## 1. 安全监视器内核调用（高）

手册表“安全监视器内核调用一览”（`Chapter/08-implementation.tex:1793`）与
`Include/rme.h:508-516` 的对照如下。

| 主功能号 | 手册 | 当前内核 | 问题 |
|---|---|---|---|
| 0xF700 | `RME_KFN_ECLV_CRT` | `RME_KFN_ECLV_CRT` | 一致 |
| 0xF701 | `RME_KFN_ECLV_DEL` | `RME_KFN_ECLV_MOD` | 名称与编号错位 |
| 0xF702 | `RME_KFN_ECLV_MOD` | `RME_KFN_ECLV_DEL` | 名称与编号错位 |
| 0xF703 | `RME_KFN_ETHD_CRT` | `RME_KFN_ECLV_ACT` | 内核无 `ETHD_*` |
| 0xF704 | `RME_KFN_ETHD_DEL` | `RME_KFN_ECLV_RET` | 内核无 `ETHD_*` |
| 0xF705 | `RME_KFN_ETHD_MOD` | 无 | 内核无对应调用 |
| 0xF706 | `RME_KFN_ETHD_ACT` | 无 | 内核无对应调用 |

手册第 8.9 节还使用 `RME_KFN_ETHD_RUN`（`Chapter/08-implementation.tex:2046`），
该名称既不在当前内核中，也不在上表的手册行中出现，手册内部也不自洽。

当前内核的安全监视器接口是：

```c
#define RME_KFN_ECLV_CRT  (0xF700U)
#define RME_KFN_ECLV_MOD  (0xF701U)
#define RME_KFN_ECLV_DEL  (0xF702U)
#define RME_KFN_ECLV_ACT  (0xF703U)
#define RME_KFN_ECLV_RET  (0xF704U)
```

影响：按手册实现的安全区线程创建/删除/运行调用会落到错误的调用号；
按内核实现时，手册中的 `ETHD_*` 调用没有实现可循。

## 2. 调试内核调用编号（高）

手册表（`Chapter/08-implementation.tex:2118-2127`）缺少当前内核的
`RME_KFN_DEBUG_EXC_GET`，导致其后编号全部错位：

| 调用 | 手册编号 | 内核编号（`Include/rme.h`） |
|---|---|---|
| `RME_KFN_DEBUG_PRINT` | 0xF800 | 0xF800 |
| `RME_KFN_DEBUG_REG_MOD` | 0xF801 | 0xF801 |
| `RME_KFN_DEBUG_INV_MOD` | 0xF802 | 0xF802 |
| `RME_KFN_DEBUG_EXC_GET` | 未记载 | 0xF803 |
| `RME_KFN_DEBUG_MODE_MOD` | 0xF803 | 0xF804 |
| `RME_KFN_DEBUG_IBP_MOD` | 0xF804 | 0xF805 |
| `RME_KFN_DEBUG_DBP_MOD` | 0xF805 | 0xF806 |

## 3. 返回码名称（中）

手册在 13 处使用 `RME_ERR_CPT_KOTBL`，当前内核定义为
`RME_ERR_CPT_KOT`（`Include/rme.h:24`）：

```c
#define RME_ERR_CPT_KOT  ((-4)+RME_ERR_CPT)
```

手册中的用法集中在“分配内核内存失败”一行，例如
`Chapter/02-capability.tex:246`、`Chapter/04-process.tex:597`、
`Chapter/07-porting.tex:1381`、`Chapter/08-implementation.tex:1416` 等。
编译当前内核的调用者若按手册书写 `RME_ERR_CPT_KOTBL`，会得到未定义标识符。

## 4. 宏与标志名称（中）

| 手册名称 | 当前内核名称 | 证据 |
|---|---|---|
| `RME_THD_MAX_TIME` | `RME_THD_INF_TIME` | `Include/Kernel/rme_kernel.h:639` |
| `RME_THD_FLAG_THD_SWT` | `RME_THD_FLAG_SWT` | `Include/rme.h:176` |
| `RME_PGT_FLAG_CON_CHILD` | `RME_PGT_FLAG_CHILD` | `Include/rme.h:118` |
| `RME_RVM_CAP_BOOT_FRONT` | `RME_RVM_CPT_BOOT_FRONT` | `Include/Platform/A7M/Chip/STM32H743ZI/rme_platform_stm32h743zi.h:46` |
| `RME_RVM_CAP_DONE_FRONT` | `RME_RVM_CPT_DONE_FRONT` | 同上，第 49 行 |
| `RME_INV_ENTRY_MAX` | 无此宏；仅有 `RME_INV_DEPTH_MAX` | `Include/Platform/A7M/rme_platform_a7m.h:118` |

`RME_THD_MAX_TIME` 在手册表 4-3 中被定义为等于 `RME_THD_INF_TIME`
（`Chapter/04-process.tex:176`），但使用处（`Chapter/04-process.tex:909`）仍写
`RME_THD_MAX_TIME`；当前内核没有这个别名。

`RME_INV_ENTRY_MAX` 只在手册的移植示例中出现
（`Chapter/07-porting.tex:270`），当前内核没有该配置项。

## 5. 服务号别名（低）

手册的服务号参数表中使用不带 `SVC_` 的旧名称，当前内核为 `RME_SVC_*`：

| 手册 | 当前内核 |
|---|---|
| `RME_INV_CRT` | `RME_SVC_INV_CRT` |
| `RME_PGT_CRT` | `RME_SVC_PGT_CRT` |
| `RME_PRC_CRT` | `RME_SVC_PRC_CRT` |
| `RME_THD_CRT` | `RME_SVC_THD_CRT` |

手册第 1 章的系统调用一览表本身使用正确的 `RME_SVC_*` 名称和 0–33 编号，
因此这是参数表中的局部旧名称，不是整体编号错误。

## 6. 移植接口名称与签名（中）

| 手册 | 当前内核 | 证据 |
|---|---|---|
| `ret_t _RME_Kotbl_Init(ptr_t Words)` | `rme_ret_t _RME_Kot_Init(rme_ptr_t Word)` | `Include/Kernel/rme_kernel.h:1301` |
| `ret_t _RME_Thd_Fatal(struct RME_Reg_Struct*, ptr_t Fault)` | `void _RME_Thd_Fatal(struct RME_Reg_Struct*)` | `Include/Kernel/rme_kernel.h:1326` |
| `cnt_t RME_Print_Int(cnt_t)` | `rme_cnt_t RME_Int_Print(rme_cnt_t)` | `Include/Kernel/rme_kernel.h:1207` |
| `cnt_t RME_Print_Hex(ptr_t)` | `rme_cnt_t RME_Hex_Print(rme_ptr_t)` | 同上，第 1206 行 |
| `cnt_t RME_Print_String(s8_t*)` | `rme_cnt_t RME_Str_Print(const rme_s8_t*)` | 同上，第 1208 行 |

另外，手册 `Chapter/07-porting.tex:2160` 的“`__RME_Thd_Cop_Check` 的实现”一节，
原型表却写成 `_RME_Thd_Cop_Init(ptr_t Attr)`；当前内核的检查函数是
`rme_ret_t __RME_Thd_Cop_Check(rme_ptr_t Attr)`
（`Include/Platform/A7M/rme_platform_a7m.h:916`），初始化函数另为
`void __RME_Thd_Cop_Init(...)`。该节标题与原型表不一致。

## 7. 手册未记载的内核返回码（低）

当前内核定义但手册未列出的返回码：

- `RME_ERR_KOT_BMP`（`Include/Kernel/rme_kernel.h:214`）
- `RME_ERR_PTH_PGT`（`Include/rme.h:52`）
- `RME_ERR_SIV_FAULT`（`Include/rme.h:73`）
- `RME_ERR_HAL_FAIL`（`Include/Kernel/rme_kernel.h:572`）
- `RME_ERR_KFN_FAIL`（`Include/Kernel/rme_kernel.h:689`）

这些返回码是否需要在手册中展开，取决于它们是否属于对外可见的接口返回值；
本报告只记录差异，不替手册补写语义。

## 8. 非缺陷的命名约定

手册在 1.4.3 节明确说明：为节省表格空间，所有 typedef 的 `rme_` 前缀均省略。
因此手册中的 `ptr_t`、`cnt_t`、`ret_t`、`cid_t`、`tid_t`、`s8_t` 等
与内核的 `rme_ptr_t`、`rme_cnt_t`、`rme_ret_t`、`rme_cid_t` 等不是不一致，
而是手册声明的排版约定。审计时不应把这一类全部计为接口错误。

同理，`RME_XXX_CPU_Local`、`RME_Timestamp`、`RME_X64_RDTSC` 等出现在
“示例”代码中的占位名称不是内核 API；`_RME_Exc_Handler` 是手册要求用户
自行命名的异常处理入口，当前内核不提供该符号也属正常。

## 9. 转换过程中已修复的内容

以下属于 DOCX 字段丢失造成的转换缺陷，已在 LaTeX 中修复，不属于内核不一致：

- Word 脚注标记（原为 `[?]`）已转换为标准 `\footnote{...}`；最终手册含 260 个脚注。
- 各“所需参数”表中的服务号、能力标志、参数名（如 `RME_SVC_CPT_CRT`、
  `RME_CPT_FLAG_CRT`、`Cap_*`）已从 R4T1 PDF 参考文本恢复。
- 各“可能返回值”表按内核/参考文本恢复了被 DOCX 丢失的参数名和返回码分组。
- 若干丢失的产品名、架构名和宏名（`RME`、`Linux`、`seL4`、`ARMv7-M`、
  `RME_CID_2L` 等）已按参考文本恢复。
- 表 3-18“构造页目录的所需参数”原先被 DOCX 合并单元格打乱，已重建。
- 版式：205 页，309 个 PDF 书签；9 章、14 幅图、338 张编号表（另有版本历史表）。

## 10. 复现方法

```bash
cd Document/Public/Manual_CN
make clean
make
```

生成物：`Document/CN_M7M01_R4T1_User_Manual.pdf`。

审计命令示例：

```bash
# 手册中出现的、但内核中不存在的 RME 标识符
python3 - <<'PY'
...
PY
```

（完整脚本可在需要时补入；本次审计未修改 `Include/` 或 `Source/`。）

## 11. 后续修复记录（2026-11-05）

按用户要求，本次只修复第 3、4、5、6 组：返回码、宏/服务号名称、移植函数签名。
第 1 组（安全监视器 `ECLV/ETHD`）、第 2 组（调试内核调用编号）和第 7 组
（未记载的内核返回码）保持原状。

### 11.1 返回码

`RME_ERR_CPT_KOTBL` 已全部改为 `RME_ERR_CPT_KOT`，共 13 处，均位于
“分配内核内存失败”一行。

### 11.2 宏与服务号

| 原手册名称 | 已改为 | 处数/说明 |
|---|---|---|
| `RME_THD_MAX_TIME` | `RME_THD_INF_TIME` | 2 处使用；同时删除了表 4-3 中重复的 `RME_THD_MAX_TIME` 行 |
| `RME_THD_FLAG_THD_SWT` | `RME_THD_FLAG_SWT` | 1 处 |
| `RME_PGT_FLAG_CON_CHILD` | `RME_PGT_FLAG_CHILD` | 1 处 |
| `RME_RVM_CAP_BOOT_FRONT`（CPT 启动行） | `RME_RVM_CPT_BOOT_FRONT` | 1 处 |
| `RME_RVM_CAP_BOOT_FRONT`（KOM 启动行） | `RME_RVM_KOM_BOOT_FRONT` | 1 处 |
| `RME_RVM_CAP_DONE_FRONT` | `RME_RVM_CPT_DONE_FRONT` | 1 处 |
| `RME_RVM_CAP_BOOT_FRONT`（KOM 完成行） | `RME_RVM_KOM_DONE_FRONT` | 1 处 |
| `RME_INV_ENTRY_MAX` | `RME_INV_DEPTH_MAX` | 1 处，即 32U 的示例 |
| `RME_INV_CRT` | `RME_SVC_INV_CRT` | 1 处 |
| `RME_PGT_CRT` | `RME_SVC_PGT_CRT` | 1 处 |
| `RME_PRC_CRT` | `RME_SVC_PRC_CRT` | 1 处 |
| `RME_THD_CRT` | `RME_SVC_THD_CRT` | 1 处 |

`RME_RVM_CAP_*` 不是全局替换：四个示例按所在行分别改为 CPT/KOM 名称；
右侧示例值（如 `RVM_CAP_NUM`）不是内核宏，未改动。

### 11.3 移植函数签名

| 原手册签名 | 已改为 | 修改内容 |
|---|---|---|
| `ret_t _RME_Kotbl_Init(ptr_t Words)` | `ret_t _RME_Kot_Init(ptr_t Word)` | 函数名、原型参数名、参数表中的 `Words` |
| `ret_t _RME_Thd_Fatal(Reg, ptr_t Fault)` | `void _RME_Thd_Fatal(struct RME_Reg_Struct* Reg)` | 返回类型改 `void`，删除 `Fault` 参数和返回值说明改为“无。” |
| `cnt_t RME_Print_Int(cnt_t Int)` | `cnt_t RME_Int_Print(cnt_t Int)` | 函数名 |
| `cnt_t RME_Print_Hex(ptr_t Uint)` | `cnt_t RME_Hex_Print(ptr_t Uint)` | 函数名 |
| `cnt_t RME_Print_String(s8_t* String)` | `cnt_t RME_Str_Print(const s8_t* String)` | 函数名；参数加 `const` |
| `ret_t _RME_Thd_Cop_Init(ptr_t Attr)`（Check 表） | `ret_t __RME_Thd_Cop_Check(ptr_t Attr)` | 只修正该表的原型；真正的 `__RME_Thd_Cop_Init` 章节未改 |

### 11.4 验证

- `make clean && make` 成功。
- 旧名称在 LaTeX 源中均已消失；新名称均可检索到。
- 无未定义引用。
- 仍为 205 页、309 个 PDF 书签。

## 12. 原稿完整性恢复（2026-11-05）

已按 R4T1 原稿逐章补齐转换时丢失的内容：

- 结构：9 章、69 节、227 个子节、75 个三级子节；原稿的 5 个被降级标题
  （4.5.4、4.5.5、7.10.5、9.1.4、9.8.7）已恢复为正式标题。
- 表格：338/338 个编号表格标题与正文一致；补齐了 7-15 的全部 typedef 示例、
  7-2 的宏示例、参数类型单元格、返回码单元格及表注。
- 图：14/14 幅正文图及标题一致。
- 脚注：260/260 条与原稿对应，无空脚注。
- 正文：逐句/逐词补齐了原稿中丢失的 RME、Linux、seL4、Composite、L4、Init、
  Cap_*、状态名、C 语言术语、宏名、架构名和指令名等。

最终手册为 205 页、309 个 PDF 书签，构建无未定义引用。

以下差异是有意保留的，不计为遗漏：此前要求的内核名称/签名修正
（`RME_ERR_CPT_KOT`、`RME_THD_INF_TIME`、`RME_THD_FLAG_SWT`、
`RME_PGT_FLAG_CHILD`、RME_RVM_CPT/KOM、`RME_INV_DEPTH_MAX`、RME_SVC_*、
`_RME_Kot_Init`、`void _RME_Thd_Fatal(Reg)`、RME_Int/Hex/Str_Print），
`RME_PGT_PHYS_ENABLE` 替代旧示例名 `RME_VA_EQU_PA`，以及已知的 KFN
ETHD/ECLV 与调试调用编号问题（属于错误，不属于漏译）。

## 13. M5P01 样式对齐（2026-11-06）

已按 M5P01 中文手册的排版约定修正：

- 338 张正文表的表头已移到 `\endhead` 之前，并在表头后加入 `\midrule`；
  跨页时表头会重复，表头与数据之间有分隔线。
- 封面标题恢复为 28/34 pt，版本行补为“版本 R4T1 · 二〇二四年九月十日”。
- 14 幅正文图统一为 `width=0.72\textwidth`；其中 12 幅使用 `[htbp]` 浮动，
  9-1、9-2 两幅改用 `[H]` 并在引导段前加 `\needspace`，避免“如下图所示”与图分页。
- 版本历史补上 R4T1（2024-09-10）一行，使版本表止于当前版本。
- 图题 `\label` 改为 M5P01 的下一行写法。
- `rme-manual.tex` 的章节数注释由“Seven-chapter”改为“Nine-chapter”。

样式文件 `rme-style.sty` 与 `rmp-style.sty` 在归一化项目名后仅余
`xltabular` 宏包差异；该宏包用于承载转换后的 338 张可换行表格。

## 14. 标签与引用样式（2026-11-06）

- 732 个结构标签已改为描述性英文 slug（`ch:overview`、`sec:4-thread-binding`、
  `tab:4-20-create-thread-parameters` 等）；不再使用纯数字标签。
- 68 个硬编码数字 `\hyperref[...]{...}` 已改为 `\ref{...}`；当前剩余 0 个。
- 9 章共 27 条参考文献均加入逐条 `\protect\phantomsection\label{cite:<cap>-<n>}`；
  正文中 55 处 `[n]` 引用已改为对应的 `\hyperref[cite:...]` 链接。
- 修正了 4.5.4/4.5.5/4.5.6/4.5.7 的标签错位；当前所有引用均可解析，构建无未定义引用。
- 表题移入 `xltabular` 内，配合 `\endfirsthead` 与 `\endhead`，使表题只出现一次、
  跨页表头重复；对仍会被分页隔开的表引导句加入了定向间距保护。

## 15. 最终版面验收（2026-11-06）

对当前 205 页 PDF 的全部 154 个含表/图的页面按 110 dpi 渲染并逐页做了视觉验收，
检查了引导句与表/图分离、孤立标题、表题与表分离、表头重复、空表头页、越界/重叠、
异常空白和图幅可读性；154/154 页通过。

本轮只调整分页与浮动位置，未改动任何正文：

- 为 55 处“该操作的返回值建议实现为如下：”引导句统一加 `\needspace{10\baselineskip}`，
  并给第 8 章各“一共有如下 N 个”小节引导段加 30 行保护，消除返回值表引导句落单。
- 2.3.4、3.3、3.3.2、3.4、4.4.3、4.5.7、4.5.9、5.5.3、7.5.2、8.2.1、8.3、
  8.5、8.6、8.7、8.7.5、8.8、8.10 等处的节标题改为
  “`\needspace{30\baselineskip}` + 标题 + `\nopagebreak[4]`”，避免标题独自留在页底。
- 删除 3.2.5 与 7.5.2 段落中间误插的 `\needspace`；5.5.1 前的分页保护在
  跨页表之后会产生空表头残页，已改为让表 5-17 正常跨页。
- 图 9-1、9-2 改为 `[H]`，并在两段“如下图所示”前加 `\needspace{14\baselineskip}`。
