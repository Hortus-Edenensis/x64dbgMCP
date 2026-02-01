# x64dbg 命令全集与 Agent Skills 超详细说明（基于官方 Commands 目录整理）

> 生成时间：2026-02-01 12:00 UTC-08:00（America/Los_Angeles）

## 说明与范围

本文目标：把 x64dbg 的 **Commands（命令行/脚本命令）** 做成“可直接落地为 Agent Skills”的超详细参考。

重要说明：
- 命令条目与分类结构以 x64dbg 官方帮助文档（help.x64dbg.com 的 Commands 目录）为主线整理。
- 为了便于做 Agent Skills，本文件在官方信息基础上补充了：**用途场景、前置条件、风险、示例、封装建议**。
- 本文尽量避免逐字复刻官方文本（版权与可维护性），而采用**中文解释 + 可操作的工程化建议**。如果你需要逐条核对细节，建议以每条命令附带的“官方页面（推断链接）”为准。

### 官方文档入口（建议收藏）

- Commands 总目录：`https://help.x64dbg.com/en/latest/commands/index.html`
- 各分类目录：
  - 通用运算/数据搬运 (General Purpose)：`https://help.x64dbg.com/en/latest/commands/general-purpose/`
  - 调试控制 (Debug Control)：`https://help.x64dbg.com/en/latest/commands/debug-control/`
  - 断点控制 (Breakpoint Control)：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/`
  - 条件断点/断点属性 (Conditional Breakpoint Control)：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/`
  - 指令跟踪 (Tracing)：`https://help.x64dbg.com/en/latest/commands/tracing/`
  - 线程控制 (Thread Control)：`https://help.x64dbg.com/en/latest/commands/thread-control/`
  - 内存操作 (Memory Operations)：`https://help.x64dbg.com/en/latest/commands/memory-operations/`
  - 操作系统控制 (Operating System Control)：`https://help.x64dbg.com/en/latest/commands/operating-system-control/`
  - 监视/Watch 控制 (Watch Control)：`https://help.x64dbg.com/en/latest/commands/watch-control/`
  - 变量 (Variables)：`https://help.x64dbg.com/en/latest/commands/variables/`
  - 搜索 (Searching)：`https://help.x64dbg.com/en/latest/commands/searching/`
  - 用户数据库/标注持久化 (User Database)：`https://help.x64dbg.com/en/latest/commands/user-database/`
  - 分析 (Analysis)：`https://help.x64dbg.com/en/latest/commands/analysis/`
  - 类型系统 (Types)：`https://help.x64dbg.com/en/latest/commands/types/`
  - 插件 (Plugins)：`https://help.x64dbg.com/en/latest/commands/plugins/`
  - 脚本命令 (Script Commands)：`https://help.x64dbg.com/en/latest/commands/script/`
  - GUI/界面控制 (GUI)：`https://help.x64dbg.com/en/latest/commands/gui/`
  - 杂项 (Miscellaneous)：`https://help.x64dbg.com/en/latest/commands/miscellaneous/`

## 命令行与脚本的共通约定（建议 Agent 先统一实现）

### 1) 表达式（Expression）与地址（VA）

x64dbg 的大多数命令参数都接受“表达式”，其本质是：能被求值为一个整数/地址的文本。常见形式：
- 十六进制/十进制常量（建议统一用十六进制显示地址）。
- 寄存器名（如 rax/eax/rcx…；以及 x64dbg 的一些兼容别名）。
- 符号/模块名+偏移（例如 `module+0x1234`，或 `module.export` 这类写法，具体取决于符号加载情况）。
- 内存解引用（例如 `[addr]`、`[reg+offset]`；注意读写权限）。

实践建议：
- Agent 输出命令时，**优先用明确的 VA**（而不是列表索引），避免 UI 列表变化导致脚本不稳定。
- 对“可写入目标（dest）”参数，必须确保它是“可赋值对象”（寄存器/变量/可写内存）。

### 2) 返回值/输出的通用习惯

- 很多命令会通过 `$result` 反馈成功/失败或计算结果。
- 搜索/引用类命令通常会把结果写到 GUI 的结果列表/引用视图；若你需要脚本读取，考虑配合 `refget` 等命令。
- 对“写入/破坏性”命令，Agent 应在执行前后记录关键状态（例如：CIP、线程、关键寄存器、目标地址原值/新值），方便回滚与审计。

### 3) Agent 安全与稳定性总则（强烈建议）

- **默认只在暂停态做写操作**（改寄存器/写内存/改断点属性/清空 DB 等）。
- 对高影响命令（StopDebug、killthread、handleclose、setpagerights、bpclear/dbclear/ClearTypes 等），建议提供：
  - `dry_run`（只输出将要执行的命令，不真正执行）；
  - `require_confirm`（需要用户显式确认）；
  - `rollback_plan`（如果可行，先备份再改）。
- 对输出到文件的命令（savedata/minidump/dbsave/export 等），建议统一管理输出目录：先 `chd` 到固定工作目录，再用相对路径写入。

## 分类总览（按官方 Commands 目录）

| Category | 命令数（本文件统计） |
|---|---:|
| 通用运算/数据搬运 (General Purpose) | 28 |
| 调试控制 (Debug Control) | 21 |
| 断点控制 (Breakpoint Control) | 28 |
| 条件断点/断点属性 (Conditional Breakpoint Control) | 55 |
| 指令跟踪 (Tracing) | 13 |
| 线程控制 (Thread Control) | 9 |
| 内存操作 (Memory Operations) | 8 |
| 操作系统控制 (Operating System Control) | 4 |
| 监视/Watch 控制 (Watch Control) | 6 |
| 变量 (Variables) | 3 |
| 搜索 (Searching) | 10 |
| 用户数据库/标注持久化 (User Database) | 23 |
| 分析 (Analysis) | 16 |
| 类型系统 (Types) | 34 |
| 插件 (Plugins) | 3 |
| 脚本命令 (Script Commands) | 15 |
| GUI/界面控制 (GUI) | 18 |
| 杂项 (Miscellaneous) | 15 |

下面按分类展开。每条命令都给出：用途、语法、参数、前置条件、返回、风险、示例、Agent 封装建议。

## 通用运算/数据搬运 (General Purpose)

本组命令更像“脚本/表达式层”的通用运算与数据搬运指令：加减乘除、位运算、移位旋转、push/pop、cmp/test、以及少量向量/掩码搬运（SSE/AVX/AVX-512 相关）。

它们常用于：
- 在调试暂停时快速计算、修改寄存器/变量/内存中的值；
- 在脚本里做控制流判断（配合 cmp/test/IFxx/Jxx）；
- 做简单的补丁（patch）或数据准备（比如把某地址写成期望值）。

### 本分类命令索引

- [add](#x64dbg-general_purpose-add) — 将两个值相加并写回到目标（arg1 = arg1 + arg2）。  （别名：add）
- [and](#x64dbg-general_purpose-and) — 按位与（arg1 = arg1 & arg2）。  （别名：and）
- [bswap](#x64dbg-general_purpose-bswap) — 对目标值做字节序反转（endian swap）。  （别名：bswap）
- [cmp](#x64dbg-general_purpose-cmp) — 比较两个值（类似 CPU cmp），通常用于更新标志位/条件判断。  （别名：cmp）
- [dec](#x64dbg-general_purpose-dec) — 对目标值自减 1。  （别名：dec）
- [div](#x64dbg-general_purpose-div) — 除法：计算 arg1 / arg2（整数除法）并写回到 arg1。  （别名：div）
- [inc](#x64dbg-general_purpose-inc) — 对目标值自增 1。  （别名：inc）
- [kmovd](#x64dbg-general_purpose-kmovd) — 移动 AVX-512 掩码寄存器数据（kmovd）。  （别名：kmovd, kmovq）
- [lzcnt](#x64dbg-general_purpose-lzcnt) — 统计前导 0 的数量（leading zero count）。  （别名：lzcnt）
- [mov](#x64dbg-general_purpose-mov) — 把源值复制到目标（赋值）。  （别名：mov, set）
- [movdqu](#x64dbg-general_purpose-movdqu) — 移动 128-bit 数据（SSE：movdqu）。  （别名：movdqu, movups, movupd）
- [mul](#x64dbg-general_purpose-mul) — 乘法：计算 arg1 * arg2，并把结果低位写回到 arg1。  （别名：mul）
- [mulhi](#x64dbg-general_purpose-mulhi) — 乘法：计算 arg1 * arg2，并把结果高位写回到 arg1。  （别名：mulhi）
- [neg](#x64dbg-general_purpose-neg) — 对目标值取二补码相反数（arg1 = -arg1）。  （别名：neg）
- [not](#x64dbg-general_purpose-not) — 对目标值按位取反（arg1 = ~arg1）。  （别名：not）
- [or](#x64dbg-general_purpose-or) — 按位或（arg1 = arg1 | arg2）。  （别名：or）
- [pop](#x64dbg-general_purpose-pop) — 从栈弹出值到目标（pop）。  （别名：pop）
- [popcnt](#x64dbg-general_purpose-popcnt) — 统计 1 的位数（population count）。  （别名：popcnt）
- [push](#x64dbg-general_purpose-push) — 将值压入被调试进程的栈（push）。  （别名：push）
- [rol](#x64dbg-general_purpose-rol) — 循环左移（rotate left）。  （别名：rol）
- [ror](#x64dbg-general_purpose-ror) — 循环右移（rotate right）。  （别名：ror）
- [sar](#x64dbg-general_purpose-sar) — 算术右移（shift right arithmetic）。  （别名：sar）
- [shl](#x64dbg-general_purpose-shl) — 逻辑左移（shift left）。  （别名：shl, sal）
- [shr](#x64dbg-general_purpose-shr) — 逻辑右移（shift right logical）。  （别名：shr）
- [sub](#x64dbg-general_purpose-sub) — 将两个值相减并写回到目标（arg1 = arg1 - arg2）。  （别名：sub）
- [test](#x64dbg-general_purpose-test) — 按位测试（类似 CPU test），通常用于更新标志位/条件判断。  （别名：test）
- [vmovdqu](#x64dbg-general_purpose-vmovdqu) — 移动向量数据（AVX：vmovdqu）。  （别名：vmovdqu, vmovups, vmovupd）
- [xor](#x64dbg-general_purpose-xor) — 按位异或（arg1 = arg1 ^ arg2）。  （别名：xor）

### add  <a id="x64dbg-general_purpose-add"></a>

- **Skill ID**：`x64dbg.general_purpose.add`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`add`
- **一句话用途**：将两个值相加并写回到目标（arg1 = arg1 + arg2）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/add.html`

#### 什么时候用（Use cases）
- 在脚本里进行算术运算并写回目标（例如计算偏移、长度、hash 中间量）。
- 对寄存器或变量做一次性修正（patch 运行时状态）。
- 做地址计算：base+offset、ptr+index*stride（配合 shl 等）。
- div 常用于把一个值按块大小分组（注意除零）。

#### 语法（Syntax）

`add <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `add rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### and  <a id="x64dbg-general_purpose-and"></a>

- **Skill ID**：`x64dbg.general_purpose.and`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`and`
- **一句话用途**：按位与（arg1 = arg1 & arg2）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/and.html`

#### 什么时候用（Use cases）
- 位级处理：掩码/置位/清位（and/or）。
- 简单混淆或校验流程复现（xor）。
- 取反/求补（neg/not）以匹配目标算法中间步骤。
- 把标志位字段提取或清理（例如 value & 0xFF）。

#### 语法（Syntax）

`and <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `and rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bswap  <a id="x64dbg-general_purpose-bswap"></a>

- **Skill ID**：`x64dbg.general_purpose.bswap`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`bswap`
- **一句话用途**：对目标值做字节序反转（endian swap）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/bswap.html`

#### 什么时候用（Use cases）
- 在大小端之间转换（例如网络字节序/文件格式字段）。
- 处理从内存读出的多字节整数时快速翻转字节序。
- 在脚本里复现协议/序列化逻辑时使用。

#### 语法（Syntax）

`bswap <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `bswap rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### cmp  <a id="x64dbg-general_purpose-cmp"></a>

- **Skill ID**：`x64dbg.general_purpose.cmp`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`cmp`
- **一句话用途**：比较两个值（类似 CPU cmp），通常用于更新标志位/条件判断。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/cmp.html`

#### 什么时候用（Use cases）
- 对两个表达式做比较/测试，生成布尔/比较结果供脚本分支使用。
- 在断点命中脚本里快速判断寄存器/内存字段是否满足条件。
- 配合 IFxx/Jxx 形成“无 GUI”自动化调试流程。

#### 语法（Syntax）

`cmp <lhs>, <rhs>`

#### 参数详解（Arguments）
- **`lhs`**（必需）：
  左操作数表达式。
- **`rhs`**（必需）：
  右操作数表达式。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `cmp rax, rbx`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### dec  <a id="x64dbg-general_purpose-dec"></a>

- **Skill ID**：`x64dbg.general_purpose.dec`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`dec`
- **一句话用途**：对目标值自减 1。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/dec.html`

#### 什么时候用（Use cases）
- 快速调整计数器/循环变量（例如对寄存器/变量自增、自减）。
- 在暂停态下“微调”某个内存字段（例如把长度+1 或 -1）。
- 配合 cmp/test/IFxx 做循环控制或阈值判断。

#### 语法（Syntax）

`dec <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `dec rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### div  <a id="x64dbg-general_purpose-div"></a>

- **Skill ID**：`x64dbg.general_purpose.div`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`div`
- **一句话用途**：除法：计算 arg1 / arg2（整数除法）并写回到 arg1。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/div.html`

#### 什么时候用（Use cases）
- 在脚本里进行算术运算并写回目标（例如计算偏移、长度、hash 中间量）。
- 对寄存器或变量做一次性修正（patch 运行时状态）。
- 做地址计算：base+offset、ptr+index*stride（配合 shl 等）。
- div 常用于把一个值按块大小分组（注意除零）。

#### 语法（Syntax）

`div <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `div rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### inc  <a id="x64dbg-general_purpose-inc"></a>

- **Skill ID**：`x64dbg.general_purpose.inc`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`inc`
- **一句话用途**：对目标值自增 1。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/inc.html`

#### 什么时候用（Use cases）
- 快速调整计数器/循环变量（例如对寄存器/变量自增、自减）。
- 在暂停态下“微调”某个内存字段（例如把长度+1 或 -1）。
- 配合 cmp/test/IFxx 做循环控制或阈值判断。

#### 语法（Syntax）

`inc <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `inc rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### kmovd  <a id="x64dbg-general_purpose-kmovd"></a>

- **Skill ID**：`x64dbg.general_purpose.kmovd`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令/别名**：`kmovd`（别名：kmovq）
- **一句话用途**：移动 AVX-512 掩码寄存器数据（kmovd）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/kmovd.html`

#### 什么时候用（Use cases）
- 把值从 src 复制到 dest（mov/set）。
- 把内存块在寄存器/内存之间搬运（movdqu/vmovdqu 等向量搬运）。
- 在调试中快速初始化结构体/缓冲区内容（配合 memset/memcpy）。

#### 语法（Syntax）

`kmovd <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `kmovd rax, 1`
- `kmovq rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### lzcnt  <a id="x64dbg-general_purpose-lzcnt"></a>

- **Skill ID**：`x64dbg.general_purpose.lzcnt`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`lzcnt`
- **一句话用途**：统计前导 0 的数量（leading zero count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/lzcnt.html`

#### 什么时候用（Use cases）
- bitcount/leading-zero 相关算法还原（例如位图、压缩、hash）。
- 快速计算一个掩码中置位数量（popcnt）。
- 快速定位最高有效位/位宽（lzcnt 可用于算 log2 近似）。

#### 语法（Syntax）

`lzcnt <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `lzcnt rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### mov  <a id="x64dbg-general_purpose-mov"></a>

- **Skill ID**：`x64dbg.general_purpose.mov`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令/别名**：`mov`（别名：set）
- **一句话用途**：把源值复制到目标（赋值）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/mov.html`

#### 什么时候用（Use cases）
- 把值从 src 复制到 dest（mov/set）。
- 把内存块在寄存器/内存之间搬运（movdqu/vmovdqu 等向量搬运）。
- 在调试中快速初始化结构体/缓冲区内容（配合 memset/memcpy）。

#### 语法（Syntax）

`mov <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `mov rax, 1`
- `set rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### movdqu  <a id="x64dbg-general_purpose-movdqu"></a>

- **Skill ID**：`x64dbg.general_purpose.movdqu`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令/别名**：`movdqu`（别名：movups, movupd）
- **一句话用途**：移动 128-bit 数据（SSE：movdqu）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/movdqu.html`

#### 什么时候用（Use cases）
- 把值从 src 复制到 dest（mov/set）。
- 把内存块在寄存器/内存之间搬运（movdqu/vmovdqu 等向量搬运）。
- 在调试中快速初始化结构体/缓冲区内容（配合 memset/memcpy）。

#### 语法（Syntax）

`movdqu <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `movdqu rax, 1`
- `movupd rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### mul  <a id="x64dbg-general_purpose-mul"></a>

- **Skill ID**：`x64dbg.general_purpose.mul`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`mul`
- **一句话用途**：乘法：计算 arg1 * arg2，并把结果低位写回到 arg1。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/mul.html`

#### 什么时候用（Use cases）
- 在脚本里进行算术运算并写回目标（例如计算偏移、长度、hash 中间量）。
- 对寄存器或变量做一次性修正（patch 运行时状态）。
- 做地址计算：base+offset、ptr+index*stride（配合 shl 等）。
- div 常用于把一个值按块大小分组（注意除零）。

#### 语法（Syntax）

`mul <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `mul rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### mulhi  <a id="x64dbg-general_purpose-mulhi"></a>

- **Skill ID**：`x64dbg.general_purpose.mulhi`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`mulhi`
- **一句话用途**：乘法：计算 arg1 * arg2，并把结果高位写回到 arg1。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/mulhi.html`

#### 什么时候用（Use cases）
- 在脚本里进行算术运算并写回目标（例如计算偏移、长度、hash 中间量）。
- 对寄存器或变量做一次性修正（patch 运行时状态）。
- 做地址计算：base+offset、ptr+index*stride（配合 shl 等）。
- div 常用于把一个值按块大小分组（注意除零）。

#### 语法（Syntax）

`mulhi <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `mulhi rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### neg  <a id="x64dbg-general_purpose-neg"></a>

- **Skill ID**：`x64dbg.general_purpose.neg`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`neg`
- **一句话用途**：对目标值取二补码相反数（arg1 = -arg1）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/neg.html`

#### 什么时候用（Use cases）
- 位级处理：掩码/置位/清位（and/or）。
- 简单混淆或校验流程复现（xor）。
- 取反/求补（neg/not）以匹配目标算法中间步骤。
- 把标志位字段提取或清理（例如 value & 0xFF）。

#### 语法（Syntax）

`neg <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `neg rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### not  <a id="x64dbg-general_purpose-not"></a>

- **Skill ID**：`x64dbg.general_purpose.not`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`not`
- **一句话用途**：对目标值按位取反（arg1 = ~arg1）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/not.html`

#### 什么时候用（Use cases）
- 位级处理：掩码/置位/清位（and/or）。
- 简单混淆或校验流程复现（xor）。
- 取反/求补（neg/not）以匹配目标算法中间步骤。
- 把标志位字段提取或清理（例如 value & 0xFF）。

#### 语法（Syntax）

`not <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `not rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### or  <a id="x64dbg-general_purpose-or"></a>

- **Skill ID**：`x64dbg.general_purpose.or`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`or`
- **一句话用途**：按位或（arg1 = arg1 | arg2）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/or.html`

#### 什么时候用（Use cases）
- 位级处理：掩码/置位/清位（and/or）。
- 简单混淆或校验流程复现（xor）。
- 取反/求补（neg/not）以匹配目标算法中间步骤。
- 把标志位字段提取或清理（例如 value & 0xFF）。

#### 语法（Syntax）

`or <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `or rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### pop  <a id="x64dbg-general_purpose-pop"></a>

- **Skill ID**：`x64dbg.general_purpose.pop`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`pop`
- **一句话用途**：从栈弹出值到目标（pop）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/pop.html`

#### 什么时候用（Use cases）
- 模拟真实 CPU push/pop 对栈的影响（快速构造参数、回放函数调用现场）。
- 在暂停态下修正栈内容（例如插入/弹出一个返回地址或参数）。
- 配合 call/ret 或手工构造 stack frame 做高级调试。

#### 语法（Syntax）

`pop <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `pop rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### popcnt  <a id="x64dbg-general_purpose-popcnt"></a>

- **Skill ID**：`x64dbg.general_purpose.popcnt`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`popcnt`
- **一句话用途**：统计 1 的位数（population count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/popcnt.html`

#### 什么时候用（Use cases）
- bitcount/leading-zero 相关算法还原（例如位图、压缩、hash）。
- 快速计算一个掩码中置位数量（popcnt）。
- 快速定位最高有效位/位宽（lzcnt 可用于算 log2 近似）。

#### 语法（Syntax）

`popcnt <dest>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `popcnt rax`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### push  <a id="x64dbg-general_purpose-push"></a>

- **Skill ID**：`x64dbg.general_purpose.push`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`push`
- **一句话用途**：将值压入被调试进程的栈（push）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/push.html`

#### 什么时候用（Use cases）
- 模拟真实 CPU push/pop 对栈的影响（快速构造参数、回放函数调用现场）。
- 在暂停态下修正栈内容（例如插入/弹出一个返回地址或参数）。
- 配合 call/ret 或手工构造 stack frame 做高级调试。

#### 语法（Syntax）

`push <value>`

#### 参数详解（Arguments）
- **`value`**（必需）：
  通用数值参数。可以是常量、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `push 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### rol  <a id="x64dbg-general_purpose-rol"></a>

- **Skill ID**：`x64dbg.general_purpose.rol`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`rol`
- **一句话用途**：循环左移（rotate left）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/rol.html`

#### 什么时候用（Use cases）
- 位移/旋转用于编码、hash、位域打包等算法还原。
- 通过位移实现乘/除 2^n 的快速计算（shl/shr/sar）。
- sar 用于有符号右移（保留符号位）。
- rol/ror 用于循环移位（常见于加密/校验）。

#### 语法（Syntax）

`rol <dest>, <count>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`count`**（必需）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `rol rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ror  <a id="x64dbg-general_purpose-ror"></a>

- **Skill ID**：`x64dbg.general_purpose.ror`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`ror`
- **一句话用途**：循环右移（rotate right）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/ror.html`

#### 什么时候用（Use cases）
- 位移/旋转用于编码、hash、位域打包等算法还原。
- 通过位移实现乘/除 2^n 的快速计算（shl/shr/sar）。
- sar 用于有符号右移（保留符号位）。
- rol/ror 用于循环移位（常见于加密/校验）。

#### 语法（Syntax）

`ror <dest>, <count>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`count`**（必需）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `ror rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### sar  <a id="x64dbg-general_purpose-sar"></a>

- **Skill ID**：`x64dbg.general_purpose.sar`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`sar`
- **一句话用途**：算术右移（shift right arithmetic）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/sar.html`

#### 什么时候用（Use cases）
- 位移/旋转用于编码、hash、位域打包等算法还原。
- 通过位移实现乘/除 2^n 的快速计算（shl/shr/sar）。
- sar 用于有符号右移（保留符号位）。
- rol/ror 用于循环移位（常见于加密/校验）。

#### 语法（Syntax）

`sar <dest>, <count>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`count`**（必需）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `sar rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### shl  <a id="x64dbg-general_purpose-shl"></a>

- **Skill ID**：`x64dbg.general_purpose.shl`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令/别名**：`shl`（别名：sal）
- **一句话用途**：逻辑左移（shift left）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/shl.html`

#### 什么时候用（Use cases）
- 位移/旋转用于编码、hash、位域打包等算法还原。
- 通过位移实现乘/除 2^n 的快速计算（shl/shr/sar）。
- sar 用于有符号右移（保留符号位）。
- rol/ror 用于循环移位（常见于加密/校验）。

#### 语法（Syntax）

`shl <dest>, <count>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`count`**（必需）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `shl rax, 1`
- `sal rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### shr  <a id="x64dbg-general_purpose-shr"></a>

- **Skill ID**：`x64dbg.general_purpose.shr`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`shr`
- **一句话用途**：逻辑右移（shift right logical）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/shr.html`

#### 什么时候用（Use cases）
- 位移/旋转用于编码、hash、位域打包等算法还原。
- 通过位移实现乘/除 2^n 的快速计算（shl/shr/sar）。
- sar 用于有符号右移（保留符号位）。
- rol/ror 用于循环移位（常见于加密/校验）。

#### 语法（Syntax）

`shr <dest>, <count>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`count`**（必需）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `shr rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### sub  <a id="x64dbg-general_purpose-sub"></a>

- **Skill ID**：`x64dbg.general_purpose.sub`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`sub`
- **一句话用途**：将两个值相减并写回到目标（arg1 = arg1 - arg2）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/sub.html`

#### 什么时候用（Use cases）
- 在脚本里进行算术运算并写回目标（例如计算偏移、长度、hash 中间量）。
- 对寄存器或变量做一次性修正（patch 运行时状态）。
- 做地址计算：base+offset、ptr+index*stride（配合 shl 等）。
- div 常用于把一个值按块大小分组（注意除零）。

#### 语法（Syntax）

`sub <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `sub rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### test  <a id="x64dbg-general_purpose-test"></a>

- **Skill ID**：`x64dbg.general_purpose.test`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`test`
- **一句话用途**：按位测试（类似 CPU test），通常用于更新标志位/条件判断。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/test.html`

#### 什么时候用（Use cases）
- 对两个表达式做比较/测试，生成布尔/比较结果供脚本分支使用。
- 在断点命中脚本里快速判断寄存器/内存字段是否满足条件。
- 配合 IFxx/Jxx 形成“无 GUI”自动化调试流程。

#### 语法（Syntax）

`test <lhs>, <rhs>`

#### 参数详解（Arguments）
- **`lhs`**（必需）：
  左操作数表达式。
- **`rhs`**（必需）：
  右操作数表达式。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `test rax, rbx`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### vmovdqu  <a id="x64dbg-general_purpose-vmovdqu"></a>

- **Skill ID**：`x64dbg.general_purpose.vmovdqu`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令/别名**：`vmovdqu`（别名：vmovups, vmovupd）
- **一句话用途**：移动向量数据（AVX：vmovdqu）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/vmovdqu.html`

#### 什么时候用（Use cases）
- 把值从 src 复制到 dest（mov/set）。
- 把内存块在寄存器/内存之间搬运（movdqu/vmovdqu 等向量搬运）。
- 在调试中快速初始化结构体/缓冲区内容（配合 memset/memcpy）。

#### 语法（Syntax）

`vmovdqu <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `vmovdqu rax, 1`
- `vmovupd rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### xor  <a id="x64dbg-general_purpose-xor"></a>

- **Skill ID**：`x64dbg.general_purpose.xor`
- **分类**：通用运算/数据搬运 (General Purpose)
- **命令**：`xor`
- **一句话用途**：按位异或（arg1 = arg1 ^ arg2）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/general-purpose/xor.html`

#### 什么时候用（Use cases）
- 位级处理：掩码/置位/清位（and/or）。
- 简单混淆或校验流程复现（xor）。
- 取反/求补（neg/not）以匹配目标算法中间步骤。
- 把标志位字段提取或清理（例如 value & 0xFF）。

#### 语法（Syntax）

`xor <dest>, <src>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `xor rax, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 调试控制 (Debug Control)

本组命令负责“调试会话生命周期 + 执行流控制”：
- 启动/停止调试、附加/分离；
- 继续运行（run/go）、暂停；
- 单步（into/over/out）以及带不同策略的扩展版本；
- 系统/用户代码边界相关的 step 变体等。

Agent 自动化时的核心原则：
1) **执行流控制命令往往是高影响操作**（会让目标进程继续跑、触发断点/异常），需要明确的前置条件（当前是否暂停、是否有待处理异常等）。
2) 建议每次 run/step 之后都做“状态采样”（例如记录 CIP、线程、异常信息、命中断点信息）。

### 本分类命令索引

- [AttachDebugger](#x64dbg-debug_control-attachdebugger) — 附加到一个正在运行的进程进行调试。  （别名：AttachDebugger, attach）
- [DebugContinue](#x64dbg-debug_control-debugcontinue) — 从调试事件/断点处继续执行（continue）。  （别名：DebugContinue, con）
- [DetachDebugger](#x64dbg-debug_control-detachdebugger) — 从当前已附加的进程中分离调试器。  （别名：DetachDebugger, detach）
- [erun](#x64dbg-debug_control-erun) — “增强运行”模式的继续执行（常用于与异常/事件相关的运行控制）。  （别名：erun, ego, er, eg）
- [eStepInto](#x64dbg-debug_control-estepinto) — 扩展单步步入（extended step into）。  （别名：eStepInto, esti）
- [eStepOut](#x64dbg-debug_control-estepout) — 扩展 StepOut（extended run-to-return）。  （别名：eStepOut, ertr）
- [eStepOver](#x64dbg-debug_control-estepover) — 扩展单步步过（extended step over）。  （别名：eStepOver, estep, esto, est）
- [InitDebug](#x64dbg-debug_control-initdebug) — 启动对一个可执行文件的调试，并在入口点暂停。  （别名：InitDebug, initdbg, init）
- [InstrUndo](#x64dbg-debug_control-instrundo) — 撤销/回退上一步指令效果（如果支持）。  （别名：InstrUndo）
- [pause](#x64dbg-debug_control-pause) — 暂停被调试进程（break/pause）。  （别名：pause）
- [run](#x64dbg-debug_control-run) — 继续/开始运行被调试进程（从当前暂停处执行）。  （别名：run, go, r, g）
- [serun](#x64dbg-debug_control-serun) — “超级增强运行”模式的继续执行（更激进的运行控制）。  （别名：serun, sego）
- [seStepInto](#x64dbg-debug_control-sestepinto) — 更进一步的扩展单步步入（super-extended）。  （别名：seStepInto, sesti）
- [seStepOver](#x64dbg-debug_control-sestepover) — 更进一步的扩展单步步过（super-extended）。  （别名：seStepOver, sestep, sesto, sest）
- [skip](#x64dbg-debug_control-skip) — 跳过当前指令：通常会把 RIP/EIP 前移到下一条指令（不执行当前指令）。  （别名：skip）
- [StepInto](#x64dbg-debug_control-stepinto) — 单步步入（step into）。  （别名：StepInto, sti）
- [StepOut](#x64dbg-debug_control-stepout) — 运行到当前函数返回（step out / run to return）。  （别名：StepOut, rtr）
- [StepOver](#x64dbg-debug_control-stepover) — 单步步过（step over）。  （别名：StepOver, step, sto, st）
- [StepSystem](#x64dbg-debug_control-stepsystem) — 单步到系统代码（与 StepUser 相对）。  （别名：StepSystem, StepSystemInto）
- [StepUser](#x64dbg-debug_control-stepuser) — 单步到用户代码（尝试跳过系统/库代码）。  （别名：StepUser, StepUserInto）
- [StopDebug](#x64dbg-debug_control-stopdebug) — 停止当前调试会话。  （别名：StopDebug, stop, dbgstop）

### AttachDebugger  <a id="x64dbg-debug_control-attachdebugger"></a>

- **Skill ID**：`x64dbg.debug_control.attachdebugger`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`AttachDebugger`（别名：attach）
- **一句话用途**：附加到一个正在运行的进程进行调试。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/attachdebugger.html`

#### 什么时候用（Use cases）
- 附加到已运行进程（例如服务、需要先启动到特定状态的程序）。
- 对短生命周期进程做“先启动后附加”式调试。
- 与 EnablePrivilege（SeDebugPrivilege）组合用于附加高权限进程。

#### 语法（Syntax）

`AttachDebugger <pid_or_name>`

#### 参数详解（Arguments）
- **`pid_or_name`**（必需）：
  进程标识：
  - PID（十进制更直观）；
  - 或进程名（可能匹配多个实例，存在歧义）。
  建议优先 PID。

#### 执行上下文 / 前置条件（Preconditions）
- 需要具备足够权限附加目标（必要时先 EnablePrivilege SeDebugPrivilege）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AttachDebugger 1234`
- `attach 1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DebugContinue  <a id="x64dbg-debug_control-debugcontinue"></a>

- **Skill ID**：`x64dbg.debug_control.debugcontinue`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`DebugContinue`（别名：con）
- **一句话用途**：从调试事件/断点处继续执行（continue）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/debugcontinue.html`

#### 什么时候用（Use cases）
- 从暂停态继续执行，直到遇到断点/异常/事件。
- 在脚本里实现“跑到某个断点/条件”的自动化流程。
- DebugContinue 常用于处理调试事件后继续。

#### 语法（Syntax）

`DebugContinue`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `DebugContinue`
- `con`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DetachDebugger  <a id="x64dbg-debug_control-detachdebugger"></a>

- **Skill ID**：`x64dbg.debug_control.detachdebugger`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`DetachDebugger`（别名：detach）
- **一句话用途**：从当前已附加的进程中分离调试器。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/detachdebugger.html`

#### 什么时候用（Use cases）
- 在不结束目标进程的情况下退出调试（让目标继续正常运行）。
- 在一轮分析/patch 后快速释放调试器占用。
- 与脚本批量附加-分析-分离流程结合。

#### 语法（Syntax）

`DetachDebugger`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DetachDebugger`
- `detach`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### erun  <a id="x64dbg-debug_control-erun"></a>

- **Skill ID**：`x64dbg.debug_control.erun`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`erun`（别名：ego, er, eg）
- **一句话用途**：“增强运行”模式的继续执行（常用于与异常/事件相关的运行控制）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/erun.html`

#### 什么时候用（Use cases）
- 从暂停态继续执行，直到遇到断点/异常/事件。
- 在脚本里实现“跑到某个断点/条件”的自动化流程。
- DebugContinue 常用于处理调试事件后继续。

#### 语法（Syntax）

`erun`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `erun`
- `eg`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### eStepInto  <a id="x64dbg-debug_control-estepinto"></a>

- **Skill ID**：`x64dbg.debug_control.estepinto`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`eStepInto`（别名：esti）
- **一句话用途**：扩展单步步入（extended step into）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/estepinto.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`eStepInto`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `eStepInto`
- `esti`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### eStepOut  <a id="x64dbg-debug_control-estepout"></a>

- **Skill ID**：`x64dbg.debug_control.estepout`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`eStepOut`（别名：ertr）
- **一句话用途**：扩展 StepOut（extended run-to-return）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/estepout.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`eStepOut`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `eStepOut`
- `ertr`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### eStepOver  <a id="x64dbg-debug_control-estepover"></a>

- **Skill ID**：`x64dbg.debug_control.estepover`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`eStepOver`（别名：estep, esto, est）
- **一句话用途**：扩展单步步过（extended step over）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/estepover.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`eStepOver`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `eStepOver`
- `est`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### InitDebug  <a id="x64dbg-debug_control-initdebug"></a>

- **Skill ID**：`x64dbg.debug_control.initdebug`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`InitDebug`（别名：initdbg, init）
- **一句话用途**：启动对一个可执行文件的调试，并在入口点暂停。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/initdebug.html`

#### 什么时候用（Use cases）
- 从磁盘启动一个可执行文件并进入调试（通常在入口点暂停）。
- 为自动化逆向脚本初始化一个新的调试会话。
- 配合 setcommandline、环境配置、断点设置，实现“一键复现”启动流程。

#### 语法（Syntax）

`InitDebug <path_to_exe>[, <cmdline>]`

#### 参数详解（Arguments）
- **`path_to_exe`**（必需）：
  要启动调试的可执行文件路径。建议使用绝对路径；包含空格时用引号包裹。
- **`cmdline`**（可选）：
  传给目标进程的命令行参数字符串（不含可执行文件本身）。建议整段用引号包裹，内部引号做好转义。

#### 执行上下文 / 前置条件（Preconditions）
- 当前没有正在调试的会话；如果已有会话，请先 StopDebug 或 DetachDebugger。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `InitDebug "C:\\path\\to\\app.exe"`
- `init "C:\\path\\to\\app.exe"`
- `InitDebug "C:\\path\\to\\app.exe", "-arg1 -arg2"`
- `init "C:\\path\\to\\app.exe", "-arg1 -arg2"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### InstrUndo  <a id="x64dbg-debug_control-instrundo"></a>

- **Skill ID**：`x64dbg.debug_control.instrundo`
- **分类**：调试控制 (Debug Control)
- **命令**：`InstrUndo`
- **一句话用途**：撤销/回退上一步指令效果（如果支持）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/instrundo.html`

#### 什么时候用（Use cases）
- InstrUndo：在支持的情况下撤销上一步指令影响（用于快速回退实验）。
- skip：跳过当前指令或按策略略过某段执行（常用于绕过无关代码）。
- 在自动化中用于“试错”式探索：执行->观察->撤销/跳过。

#### 语法（Syntax）

`InstrUndo`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `InstrUndo`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### pause  <a id="x64dbg-debug_control-pause"></a>

- **Skill ID**：`x64dbg.debug_control.pause`
- **分类**：调试控制 (Debug Control)
- **命令**：`pause`
- **一句话用途**：暂停被调试进程（break/pause）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/pause.html`

#### 什么时候用（Use cases）
- 让正在运行的目标暂停到当前指令（Break）。
- 在自动化流程中“收割”运行态，进入可读可写的稳定状态。
- 配合 watchdog/watch 或外部条件触发时强制暂停。

#### 语法（Syntax）

`pause`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `pause`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### run  <a id="x64dbg-debug_control-run"></a>

- **Skill ID**：`x64dbg.debug_control.run`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`run`（别名：go, r, g）
- **一句话用途**：继续/开始运行被调试进程（从当前暂停处执行）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/run.html`

#### 什么时候用（Use cases）
- 从暂停态继续执行，直到遇到断点/异常/事件。
- 在脚本里实现“跑到某个断点/条件”的自动化流程。
- DebugContinue 常用于处理调试事件后继续。

#### 语法（Syntax）

`run`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `run`
- `g`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### serun  <a id="x64dbg-debug_control-serun"></a>

- **Skill ID**：`x64dbg.debug_control.serun`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`serun`（别名：sego）
- **一句话用途**：“超级增强运行”模式的继续执行（更激进的运行控制）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/serun.html`

#### 什么时候用（Use cases）
- 从暂停态继续执行，直到遇到断点/异常/事件。
- 在脚本里实现“跑到某个断点/条件”的自动化流程。
- DebugContinue 常用于处理调试事件后继续。

#### 语法（Syntax）

`serun`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `serun`
- `sego`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### seStepInto  <a id="x64dbg-debug_control-sestepinto"></a>

- **Skill ID**：`x64dbg.debug_control.sestepinto`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`seStepInto`（别名：sesti）
- **一句话用途**：更进一步的扩展单步步入（super-extended）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/sestepinto.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`seStepInto`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `seStepInto`
- `sesti`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### seStepOver  <a id="x64dbg-debug_control-sestepover"></a>

- **Skill ID**：`x64dbg.debug_control.sestepover`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`seStepOver`（别名：sestep, sesto, sest）
- **一句话用途**：更进一步的扩展单步步过（super-extended）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/sestepover.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`seStepOver`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `seStepOver`
- `sest`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### skip  <a id="x64dbg-debug_control-skip"></a>

- **Skill ID**：`x64dbg.debug_control.skip`
- **分类**：调试控制 (Debug Control)
- **命令**：`skip`
- **一句话用途**：跳过当前指令：通常会把 RIP/EIP 前移到下一条指令（不执行当前指令）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/skip.html`

#### 什么时候用（Use cases）
- InstrUndo：在支持的情况下撤销上一步指令影响（用于快速回退实验）。
- skip：跳过当前指令或按策略略过某段执行（常用于绕过无关代码）。
- 在自动化中用于“试错”式探索：执行->观察->撤销/跳过。

#### 语法（Syntax）

`skip`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `skip`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StepInto  <a id="x64dbg-debug_control-stepinto"></a>

- **Skill ID**：`x64dbg.debug_control.stepinto`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StepInto`（别名：sti）
- **一句话用途**：单步步入（step into）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stepinto.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`StepInto`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `StepInto`
- `sti`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StepOut  <a id="x64dbg-debug_control-stepout"></a>

- **Skill ID**：`x64dbg.debug_control.stepout`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StepOut`（别名：rtr）
- **一句话用途**：运行到当前函数返回（step out / run to return）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stepout.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`StepOut`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `StepOut`
- `rtr`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StepOver  <a id="x64dbg-debug_control-stepover"></a>

- **Skill ID**：`x64dbg.debug_control.stepover`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StepOver`（别名：step, sto, st）
- **一句话用途**：单步步过（step over）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stepover.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`StepOver`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `StepOver`
- `st`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StepSystem  <a id="x64dbg-debug_control-stepsystem"></a>

- **Skill ID**：`x64dbg.debug_control.stepsystem`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StepSystem`（别名：StepSystemInto）
- **一句话用途**：单步到系统代码（与 StepUser 相对）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stepsystem.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`StepSystem`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `StepSystem`
- `StepSystemInto`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StepUser  <a id="x64dbg-debug_control-stepuser"></a>

- **Skill ID**：`x64dbg.debug_control.stepuser`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StepUser`（别名：StepUserInto）
- **一句话用途**：单步到用户代码（尝试跳过系统/库代码）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stepuser.html`

#### 什么时候用（Use cases）
- 单步跟踪执行流，定位关键分支/调用点。
- 在不确定调用会进入哪里时用 StepInto；不想进入函数体时用 StepOver。
- StepOut/RunToReturn 用于快速回到上层调用点。
- 扩展/系统/用户版 step 通常用于控制是否进入系统模块或做额外过滤（具体以官方实现为准）。

#### 语法（Syntax）

`StepUser`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 通常需要当前处于暂停态（或处于可继续的调试事件上下文）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 会推进目标执行流，可能触发新的断点/异常/反调试逻辑，导致状态变化。

#### 示例（Examples）
- `StepUser`
- `StepUserInto`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StopDebug  <a id="x64dbg-debug_control-stopdebug"></a>

- **Skill ID**：`x64dbg.debug_control.stopdebug`
- **分类**：调试控制 (Debug Control)
- **命令/别名**：`StopDebug`（别名：stop, dbgstop）
- **一句话用途**：停止当前调试会话。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/debug-control/stopdebug.html`

#### 什么时候用（Use cases）
- 结束当前调试会话并终止/释放调试状态。
- 自动化脚本的清理步骤（确保下次 InitDebug 干净）。
- 当目标进入不可恢复状态（死循环/崩溃边缘）时强制结束调试。

#### 语法（Syntax）

`StopDebug`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `StopDebug`
- `stop`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 断点控制 (Breakpoint Control)

本组命令用于创建/删除/启用/禁用各类断点（软件/硬件/内存/模块加载/异常），以及列出断点/定位断点。

自动化要点：
- **优先用地址管理软件断点**，用“槽位”管理硬件断点；
- 对内存断点要考虑页面权限和性能开销；
- 对异常断点要谨慎：某些异常在正常流程里频繁发生，容易造成“断点风暴”。

### 本分类命令索引

- [bpclear](#x64dbg-breakpoint_control-bpclear) — 清除所有断点。  （别名：bpclear）
- [bpgoto](#x64dbg-breakpoint_control-bpgoto) — 跳转/定位到某个断点（通常用于 GUI 定位或把 CIP 设置到断点地址）。  （别名：bpgoto）
- [bplist](#x64dbg-breakpoint_control-bplist) — 列出当前所有断点（软件/硬件/内存）。  （别名：bplist）
- [DeleteBPX](#x64dbg-breakpoint_control-deletebpx) — 删除/清除软件断点。  （别名：DeleteBPX, bc, bpc）
- [DeleteExceptionBPX](#x64dbg-breakpoint_control-deleteexceptionbpx) — 删除异常断点。  （别名：DeleteExceptionBPX）
- [DeleteHardwareBreakpoint](#x64dbg-breakpoint_control-deletehardwarebreakpoint) — 删除硬件断点。  （别名：DeleteHardwareBreakpoint, bphc, bphwc, DeleteHWBPX）
- [DeleteMemoryBreakpoint](#x64dbg-breakpoint_control-deletememorybreakpoint) — 删除内存断点。  （别名：DeleteMemoryBreakpoint, bpmc, DeleteMemBPX, DeleteMemoryBPX, membpc）
- [DisableBPX](#x64dbg-breakpoint_control-disablebpx) — 禁用软件断点。  （别名：DisableBPX, bd, bpd）
- [DisableExceptionBPX](#x64dbg-breakpoint_control-disableexceptionbpx) — 禁用异常断点。  （别名：DisableExceptionBPX）
- [DisableHardwareBreakpoint](#x64dbg-breakpoint_control-disablehardwarebreakpoint) — 禁用硬件断点。  （别名：DisableHardwareBreakpoint, bphd, bphwd, DisableHWBPX）
- [DisableMemoryBreakpoint](#x64dbg-breakpoint_control-disablememorybreakpoint) — 禁用内存断点。  （别名：DisableMemoryBreakpoint, bpmd, DisableMemBPX, membpd）
- [EnableBPX](#x64dbg-breakpoint_control-enablebpx) — 启用软件断点。  （别名：EnableBPX, be, bpe）
- [EnableExceptionBPX](#x64dbg-breakpoint_control-enableexceptionbpx) — 启用异常断点。  （别名：EnableExceptionBPX）
- [EnableHardwareBreakpoint](#x64dbg-breakpoint_control-enablehardwarebreakpoint) — 启用硬件断点。  （别名：EnableHardwareBreakpoint, bphe, bphwe, EnableHWBPX）
- [EnableMemoryBreakpoint](#x64dbg-breakpoint_control-enablememorybreakpoint) — 启用内存断点。  （别名：EnableMemoryBreakpoint, bpme, EnableMemBPX, membpe）
- [LibrarianDisableBreakpoint](#x64dbg-breakpoint_control-librariandisablebreakpoint) — 禁用 Librarian 断点。  （别名：LibrarianDisableBreakpoint, bpddll）
- [LibrarianEnableBreakpoint](#x64dbg-breakpoint_control-librarianenablebreakpoint) — 启用 Librarian 断点。  （别名：LibrarianEnableBreakpoint, bpedll）
- [LibrarianRemoveBreakpoint](#x64dbg-breakpoint_control-librarianremovebreakpoint) — 移除 Librarian 断点。  （别名：LibrarianRemoveBreakpoint, bcdll）
- [LibrarianSetBreakpoint](#x64dbg-breakpoint_control-librariansetbreakpoint) — 设置 Librarian（模块/库加载相关）断点。  （别名：LibrarianSetBreakpoint, bpdll）
- [SetBPX](#x64dbg-breakpoint_control-setbpx) — 设置软件断点（INT3）到指定地址。  （别名：SetBPX, bp, bpx）
- [SetBPXOptions](#x64dbg-breakpoint_control-setbpxoptions) — 设置断点选项（类型/大小/地址/函数/静默/模式等）。  （别名：SetBPXOptions, bpaddr, bpfun, bpmode, bpsilent, bpsize, bptype）
- [SetExceptionBPX](#x64dbg-breakpoint_control-setexceptionbpx) — 设置异常断点（按异常代码/类型触发）。  （别名：SetExceptionBPX）
- [SetHardwareBreakpoint](#x64dbg-breakpoint_control-sethardwarebreakpoint) — 设置硬件断点（HWBP）。  （别名：SetHardwareBreakpoint, SetHWBPX, bph, bphws）
- [SetHardwareBreakpointOptions](#x64dbg-breakpoint_control-sethardwarebreakpointoptions) — 设置断点选项（类型/大小/地址/函数/静默/模式等）。  （别名：SetHardwareBreakpointOptions, bphwtype, bphwsize）
- [SetMemoryBreakpoint](#x64dbg-breakpoint_control-setmemorybreakpoint) — 设置内存断点（访问/写/执行等）。  （别名：SetMemoryBreakpoint, bpm, membp, SetMemBPX, SetMemoryBPX）
- [SetMemoryBreakpointOptions](#x64dbg-breakpoint_control-setmemorybreakpointoptions) — 设置断点选项（类型/大小/地址/函数/静默/模式等）。  （别名：SetMemoryBreakpointOptions, bpmsize, bpmtype）
- [SetMemoryRangeBPX](#x64dbg-breakpoint_control-setmemoryrangebpx) — 在一段地址范围上设置内存断点（范围版）。  （别名：SetMemoryRangeBPX, memrangebp, bpmrange）
- [ToggleBPX](#x64dbg-breakpoint_control-togglebpx) — 切换（启用/禁用）软件断点。  （别名：ToggleBPX, bt）

### bpclear  <a id="x64dbg-breakpoint_control-bpclear"></a>

- **Skill ID**：`x64dbg.breakpoint_control.bpclear`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`bpclear`
- **一句话用途**：清除所有断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/bpclear.html`

#### 什么时候用（Use cases）
- 清理不再需要的断点，避免误触发或性能下降。
- 自动化脚本结束时做“收尾”清理，保持环境干净。
- 当断点设置错误（地址不对/范围太大）时快速撤销。

#### 语法（Syntax）

`bpclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 清空操作不可逆（除非你事先备份 DB/类型/配置）。

#### 示例（Examples）
- `bpclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bpgoto  <a id="x64dbg-breakpoint_control-bpgoto"></a>

- **Skill ID**：`x64dbg.breakpoint_control.bpgoto`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`bpgoto`
- **一句话用途**：跳转/定位到某个断点（通常用于 GUI 定位或把 CIP 设置到断点地址）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/bpgoto.html`

#### 什么时候用（Use cases）
- 在 GUI 中快速定位到某个断点对应地址。
- 把当前反汇编/当前执行位置跳转到断点处便于查看上下文。
- 结合 bplist -> bpgoto 做交互式断点导航。

#### 语法（Syntax）

`bpgoto <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `bpgoto 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bplist  <a id="x64dbg-breakpoint_control-bplist"></a>

- **Skill ID**：`x64dbg.breakpoint_control.bplist`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`bplist`
- **一句话用途**：列出当前所有断点（软件/硬件/内存）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/bplist.html`

#### 什么时候用（Use cases）
- 列出当前断点配置，用于脚本决策或调试校验。
- 在自动化中把断点列表输出到日志，便于复现问题。
- 配合 addr_or_index 参数管理断点（例如 EnableBPX/DisableBPX）。

#### 语法（Syntax）

`bplist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `bplist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DeleteBPX  <a id="x64dbg-breakpoint_control-deletebpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.deletebpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DeleteBPX`（别名：bc, bpc）
- **一句话用途**：删除/清除软件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/deletebpx.html`

#### 什么时候用（Use cases）
- 清理不再需要的断点，避免误触发或性能下降。
- 自动化脚本结束时做“收尾”清理，保持环境干净。
- 当断点设置错误（地址不对/范围太大）时快速撤销。

#### 语法（Syntax）

`DeleteBPX <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DeleteBPX 401000`
- `bc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DeleteExceptionBPX  <a id="x64dbg-breakpoint_control-deleteexceptionbpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.deleteexceptionbpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`DeleteExceptionBPX`
- **一句话用途**：删除异常断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/deleteexceptionbpx.html`

#### 什么时候用（Use cases）
- 清理不再需要的断点，避免误触发或性能下降。
- 自动化脚本结束时做“收尾”清理，保持环境干净。
- 当断点设置错误（地址不对/范围太大）时快速撤销。

#### 语法（Syntax）

`DeleteExceptionBPX <exception_code_or_name>`

#### 参数详解（Arguments）
- **`exception_code_or_name`**（必需）：
  异常代码（例如 0xC0000005）或异常名称（如果实现支持）。用于设置/管理异常断点。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DeleteExceptionBPX 0xC0000005`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DeleteHardwareBreakpoint  <a id="x64dbg-breakpoint_control-deletehardwarebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.deletehardwarebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DeleteHardwareBreakpoint`（别名：bphc, bphwc, DeleteHWBPX）
- **一句话用途**：删除硬件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/deletehardwarebreakpoint.html`

#### 什么时候用（Use cases）
- 清理不再需要的断点，避免误触发或性能下降。
- 自动化脚本结束时做“收尾”清理，保持环境干净。
- 当断点设置错误（地址不对/范围太大）时快速撤销。

#### 语法（Syntax）

`DeleteHardwareBreakpoint <slot_or_addr>`

#### 参数详解（Arguments）
- **`slot_or_addr`**（必需）：
  硬件断点槽位或地址：
  - 槽位一般是 0~3（x86/x64 硬件断点寄存器数量有限）；
  - 或直接给当初设置的地址（由实现决定）。
  建议 Agent 优先按槽位管理，避免同址多断点歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DeleteHardwareBreakpoint 0`
- `bphc 0`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DeleteMemoryBreakpoint  <a id="x64dbg-breakpoint_control-deletememorybreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.deletememorybreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DeleteMemoryBreakpoint`（别名：bpmc, DeleteMemBPX, DeleteMemoryBPX, membpc）
- **一句话用途**：删除内存断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/deletememorybreakpoint.html`

#### 什么时候用（Use cases）
- 清理不再需要的断点，避免误触发或性能下降。
- 自动化脚本结束时做“收尾”清理，保持环境干净。
- 当断点设置错误（地址不对/范围太大）时快速撤销。

#### 语法（Syntax）

`DeleteMemoryBreakpoint <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DeleteMemoryBreakpoint 401000`
- `bpmc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DisableBPX  <a id="x64dbg-breakpoint_control-disablebpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.disablebpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DisableBPX`（别名：bd, bpd）
- **一句话用途**：禁用软件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/disablebpx.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`DisableBPX <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisableBPX 401000`
- `bd 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DisableExceptionBPX  <a id="x64dbg-breakpoint_control-disableexceptionbpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.disableexceptionbpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`DisableExceptionBPX`
- **一句话用途**：禁用异常断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/disableexceptionbpx.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`DisableExceptionBPX <exception_code_or_name>`

#### 参数详解（Arguments）
- **`exception_code_or_name`**（必需）：
  异常代码（例如 0xC0000005）或异常名称（如果实现支持）。用于设置/管理异常断点。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisableExceptionBPX 0xC0000005`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DisableHardwareBreakpoint  <a id="x64dbg-breakpoint_control-disablehardwarebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.disablehardwarebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DisableHardwareBreakpoint`（别名：bphd, bphwd, DisableHWBPX）
- **一句话用途**：禁用硬件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/disablehardwarebreakpoint.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`DisableHardwareBreakpoint <slot_or_addr>`

#### 参数详解（Arguments）
- **`slot_or_addr`**（必需）：
  硬件断点槽位或地址：
  - 槽位一般是 0~3（x86/x64 硬件断点寄存器数量有限）；
  - 或直接给当初设置的地址（由实现决定）。
  建议 Agent 优先按槽位管理，避免同址多断点歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisableHardwareBreakpoint 0`
- `bphd 0`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DisableMemoryBreakpoint  <a id="x64dbg-breakpoint_control-disablememorybreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.disablememorybreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`DisableMemoryBreakpoint`（别名：bpmd, DisableMemBPX, membpd）
- **一句话用途**：禁用内存断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/disablememorybreakpoint.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`DisableMemoryBreakpoint <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisableMemoryBreakpoint 401000`
- `bpmd 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnableBPX  <a id="x64dbg-breakpoint_control-enablebpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.enablebpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`EnableBPX`（别名：be, bpe）
- **一句话用途**：启用软件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/enablebpx.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`EnableBPX <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnableBPX 401000`
- `be 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnableExceptionBPX  <a id="x64dbg-breakpoint_control-enableexceptionbpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.enableexceptionbpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`EnableExceptionBPX`
- **一句话用途**：启用异常断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/enableexceptionbpx.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`EnableExceptionBPX <exception_code_or_name>`

#### 参数详解（Arguments）
- **`exception_code_or_name`**（必需）：
  异常代码（例如 0xC0000005）或异常名称（如果实现支持）。用于设置/管理异常断点。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnableExceptionBPX 0xC0000005`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnableHardwareBreakpoint  <a id="x64dbg-breakpoint_control-enablehardwarebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.enablehardwarebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`EnableHardwareBreakpoint`（别名：bphe, bphwe, EnableHWBPX）
- **一句话用途**：启用硬件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/enablehardwarebreakpoint.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`EnableHardwareBreakpoint <slot_or_addr>`

#### 参数详解（Arguments）
- **`slot_or_addr`**（必需）：
  硬件断点槽位或地址：
  - 槽位一般是 0~3（x86/x64 硬件断点寄存器数量有限）；
  - 或直接给当初设置的地址（由实现决定）。
  建议 Agent 优先按槽位管理，避免同址多断点歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnableHardwareBreakpoint 0`
- `bphe 0`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnableMemoryBreakpoint  <a id="x64dbg-breakpoint_control-enablememorybreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.enablememorybreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`EnableMemoryBreakpoint`（别名：bpme, EnableMemBPX, membpe）
- **一句话用途**：启用内存断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/enablememorybreakpoint.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`EnableMemoryBreakpoint <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnableMemoryBreakpoint 401000`
- `bpme 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### LibrarianDisableBreakpoint  <a id="x64dbg-breakpoint_control-librariandisablebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.librariandisablebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`LibrarianDisableBreakpoint`（别名：bpddll）
- **一句话用途**：禁用 Librarian 断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/librariandisablebreakpoint.html`

#### 什么时候用（Use cases）
- 断点创建/管理/查询。
- 为自动化调试提供可控的停点。
- 与条件断点、trace、日志联动。

#### 语法（Syntax）

`LibrarianDisableBreakpoint <id_or_pattern>`

#### 参数详解（Arguments）
- **`id_or_pattern`**（必需）：
  ID/索引或匹配模式（pattern）。用于按名称/模块模式选择对象。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `LibrarianDisableBreakpoint kernel32.dll`
- `bpddll kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### LibrarianEnableBreakpoint  <a id="x64dbg-breakpoint_control-librarianenablebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.librarianenablebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`LibrarianEnableBreakpoint`（别名：bpedll）
- **一句话用途**：启用 Librarian 断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/librarianenablebreakpoint.html`

#### 什么时候用（Use cases）
- 断点创建/管理/查询。
- 为自动化调试提供可控的停点。
- 与条件断点、trace、日志联动。

#### 语法（Syntax）

`LibrarianEnableBreakpoint <id_or_pattern>`

#### 参数详解（Arguments）
- **`id_or_pattern`**（必需）：
  ID/索引或匹配模式（pattern）。用于按名称/模块模式选择对象。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `LibrarianEnableBreakpoint kernel32.dll`
- `bpedll kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### LibrarianRemoveBreakpoint  <a id="x64dbg-breakpoint_control-librarianremovebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.librarianremovebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`LibrarianRemoveBreakpoint`（别名：bcdll）
- **一句话用途**：移除 Librarian 断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/librarianremovebreakpoint.html`

#### 什么时候用（Use cases）
- 断点创建/管理/查询。
- 为自动化调试提供可控的停点。
- 与条件断点、trace、日志联动。

#### 语法（Syntax）

`LibrarianRemoveBreakpoint <id_or_pattern>`

#### 参数详解（Arguments）
- **`id_or_pattern`**（必需）：
  ID/索引或匹配模式（pattern）。用于按名称/模块模式选择对象。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `LibrarianRemoveBreakpoint kernel32.dll`
- `bcdll kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### LibrarianSetBreakpoint  <a id="x64dbg-breakpoint_control-librariansetbreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.librariansetbreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`LibrarianSetBreakpoint`（别名：bpdll）
- **一句话用途**：设置 Librarian（模块/库加载相关）断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/librariansetbreakpoint.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`LibrarianSetBreakpoint <module_or_path_pattern>[, <options>]`

#### 参数详解（Arguments）
- **`module_or_path_pattern`**（必需）：
  模块名或路径匹配模式：
  - 常见写法是 DLL 名（kernel32.dll）；
  - 或通配符/正则风格的 pattern（取决于实现）。
  用于 Librarian 断点时通常表示“当某模块加载时触发”。
- **`options`**（可选）：
  可选项集合（通常为逗号分隔的标志位或 key=value 对）。具体可用项依命令实现而定。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `LibrarianSetBreakpoint kernel32.dll`
- `bpdll kernel32.dll`
- `LibrarianSetBreakpoint kernel32.dll, "1"`
- `bpdll kernel32.dll, "1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBPX  <a id="x64dbg-breakpoint_control-setbpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setbpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetBPX`（别名：bp, bpx）
- **一句话用途**：设置软件断点（INT3）到指定地址。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setbpx.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`SetBPX <addr>[, <name_or_options>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`name_or_options`**（可选）：
  名称或选项：
  - 如果你想给断点设置名字/备注，传入 name；
  - 如果你想一次性设置更多属性，传入 options（可能包含 name、silent、fastresume 等）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBPX 401000`
- `bp 401000`
- `SetBPX 401000, "MyBP"`
- `bp 401000, "MyBP"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBPXOptions  <a id="x64dbg-breakpoint_control-setbpxoptions"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setbpxoptions`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetBPXOptions`（别名：bpaddr, bpfun, bpmode, bpsilent, bpsize, bptype）
- **一句话用途**：设置断点选项（类型/大小/地址/函数/静默/模式等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setbpxoptions.html`

#### 什么时候用（Use cases）
- 调整已有断点的属性（类型/大小/静默/模式等）。
- 把 GUI 中能设置的断点属性脚本化，形成可复现配置。
- 与条件断点属性（Conditional Breakpoint Control）配合实现更精细控制。

#### 语法（Syntax）

`SetBPXOptions <addr_or_index>, <option_value>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。
- **`option_value`**（必需）：
  选项值。其含义取决于你通过别名选择的选项类型（例如 bptype/bpsize/bpsilent…）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBPXOptions 401000, 1`
- `bpfun 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBPX  <a id="x64dbg-breakpoint_control-setexceptionbpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setexceptionbpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令**：`SetExceptionBPX`
- **一句话用途**：设置异常断点（按异常代码/类型触发）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setexceptionbpx.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`SetExceptionBPX <exception_code_or_name>[, <options>]`

#### 参数详解（Arguments）
- **`exception_code_or_name`**（必需）：
  异常代码（例如 0xC0000005）或异常名称（如果实现支持）。用于设置/管理异常断点。
- **`options`**（可选）：
  可选项集合（通常为逗号分隔的标志位或 key=value 对）。具体可用项依命令实现而定。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBPX 0xC0000005`
- `SetExceptionBPX 0xC0000005, "1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpoint  <a id="x64dbg-breakpoint_control-sethardwarebreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.sethardwarebreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpoint`（别名：SetHWBPX, bph, bphws）
- **一句话用途**：设置硬件断点（HWBP）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/sethardwarebreakpoint.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`SetHardwareBreakpoint <addr>[, <type>, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`type`**（可选）：
  类型参数（枚举/字符串/数值）。不同命令语义不同：
  - 硬件断点常见为 exec/write/access；
  - 内存断点常见为 r/w/x 或 read/write/execute；
  - SetBPXOptions 等则是具体选项类型。
  若不确定可用值，先在 GUI 中创建一次并用 bplist/相关命令查看其表现。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpoint 401000`
- `bph 401000`
- `SetHardwareBreakpoint 401000, exec, 10`
- `bph 401000, exec, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointOptions  <a id="x64dbg-breakpoint_control-sethardwarebreakpointoptions"></a>

- **Skill ID**：`x64dbg.breakpoint_control.sethardwarebreakpointoptions`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpointOptions`（别名：bphwtype, bphwsize）
- **一句话用途**：设置断点选项（类型/大小/地址/函数/静默/模式等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/sethardwarebreakpointoptions.html`

#### 什么时候用（Use cases）
- 调整已有断点的属性（类型/大小/静默/模式等）。
- 把 GUI 中能设置的断点属性脚本化，形成可复现配置。
- 与条件断点属性（Conditional Breakpoint Control）配合实现更精细控制。

#### 语法（Syntax）

`SetHardwareBreakpointOptions <addr_or_index>, <option_value>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。
- **`option_value`**（必需）：
  选项值。其含义取决于你通过别名选择的选项类型（例如 bptype/bpsize/bpsilent…）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointOptions 401000, 1`
- `bphwsize 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpoint  <a id="x64dbg-breakpoint_control-setmemorybreakpoint"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setmemorybreakpoint`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpoint`（别名：bpm, membp, SetMemBPX, SetMemoryBPX）
- **一句话用途**：设置内存断点（访问/写/执行等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setmemorybreakpoint.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`SetMemoryBreakpoint <addr>, <size>[, <type>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。
- **`type`**（可选）：
  类型参数（枚举/字符串/数值）。不同命令语义不同：
  - 硬件断点常见为 exec/write/access；
  - 内存断点常见为 r/w/x 或 read/write/execute；
  - SetBPXOptions 等则是具体选项类型。
  若不确定可用值，先在 GUI 中创建一次并用 bplist/相关命令查看其表现。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpoint 401000, 10`
- `bpm 401000, 10`
- `SetMemoryBreakpoint 401000, 10, exec`
- `bpm 401000, 10, exec`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointOptions  <a id="x64dbg-breakpoint_control-setmemorybreakpointoptions"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setmemorybreakpointoptions`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpointOptions`（别名：bpmsize, bpmtype）
- **一句话用途**：设置断点选项（类型/大小/地址/函数/静默/模式等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setmemorybreakpointoptions.html`

#### 什么时候用（Use cases）
- 调整已有断点的属性（类型/大小/静默/模式等）。
- 把 GUI 中能设置的断点属性脚本化，形成可复现配置。
- 与条件断点属性（Conditional Breakpoint Control）配合实现更精细控制。

#### 语法（Syntax）

`SetMemoryBreakpointOptions <addr_or_index>, <option_value>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。
- **`option_value`**（必需）：
  选项值。其含义取决于你通过别名选择的选项类型（例如 bptype/bpsize/bpsilent…）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointOptions 401000, 1`
- `bpmsize 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryRangeBPX  <a id="x64dbg-breakpoint_control-setmemoryrangebpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.setmemoryrangebpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`SetMemoryRangeBPX`（别名：memrangebp, bpmrange）
- **一句话用途**：在一段地址范围上设置内存断点（范围版）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/setmemoryrangebpx.html`

#### 什么时候用（Use cases）
- 在关键地址/事件上设置断点，以截获执行流或数据访问。
- 脚本启动时预埋断点，随后 run 直到命中。
- 配合 Conditional Breakpoint 命令设置条件、日志、自动命令，实现“命中即采样”。

#### 语法（Syntax）

`SetMemoryRangeBPX <start_addr>, <end_addr_or_size>[, <type>]`

#### 参数详解（Arguments）
- **`start_addr`**（必需）：
  范围起始地址（VA）。
- **`end_addr_or_size`**（必需）：
  范围结束地址或长度：
  - 某些命令接受 end（结束地址，通常不含/含端点取决于实现）；
  - 某些命令接受 size（字节数）。
  建议 Agent 生成时同时在注释里写清楚你采用的是哪一种。
- **`type`**（可选）：
  类型参数（枚举/字符串/数值）。不同命令语义不同：
  - 硬件断点常见为 exec/write/access；
  - 内存断点常见为 r/w/x 或 read/write/execute；
  - SetBPXOptions 等则是具体选项类型。
  若不确定可用值，先在 GUI 中创建一次并用 bplist/相关命令查看其表现。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryRangeBPX 401000, 100`
- `bpmrange 401000, 100`
- `SetMemoryRangeBPX 401000, 100, exec`
- `bpmrange 401000, 100, exec`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ToggleBPX  <a id="x64dbg-breakpoint_control-togglebpx"></a>

- **Skill ID**：`x64dbg.breakpoint_control.togglebpx`
- **分类**：断点控制 (Breakpoint Control)
- **命令/别名**：`ToggleBPX`（别名：bt）
- **一句话用途**：切换（启用/禁用）软件断点。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/breakpoint-control/togglebpx.html`

#### 什么时候用（Use cases）
- 临时开关断点：在不同阶段启用不同断点组。
- 在批量 run/trace 时避免某些断点干扰。
- 调试时保留断点配置但不让它触发（Disable）。

#### 语法（Syntax）

`ToggleBPX <addr_or_index>`

#### 参数详解（Arguments）
- **`addr_or_index`**（必需）：
  断点/对象的定位符：
  - 直接给地址（VA）；
  - 或给列表索引/ID（通常来自 bplist/GUI 列表）。
  在不确定时，优先用地址，避免索引随会话变化。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ToggleBPX 401000`
- `bt 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 条件断点/断点属性 (Conditional Breakpoint Control)

本组命令用于“给断点设置高级属性”，典型包括：
- 名称（Name）/备注；
- 条件（Condition）；
- 命中日志（Log）及其条件（LogCondition）；
- 命中时要执行的命令（Command）及其条件（CommandCondition）；
- fastresume（命中后自动继续）、singleshoot（一次性）、silent（静默不弹窗）；
- hitcount 读取与清零。

这些命令通常与 Breakpoint Control（设置断点本体）配合使用：先 SetBPX/SetHardwareBreakpoint/SetMemoryBreakpoint，再设置条件与自动化动作。

### 本分类命令索引

- [GetBreakpointHitCount](#x64dbg-conditional_breakpoint_control-getbreakpointhitcount) — Conditional Breakpoint Control 分类命令：GetBreakpointHitCount（请参考官方文档确认参数细节）。  （别名：GetBreakpointHitCount）
- [GetExceptionBreakpointHitCount](#x64dbg-conditional_breakpoint_control-getexceptionbreakpointhitcount) — 获取异常断点命中次数（hit count）。  （别名：GetExceptionBreakpointHitCount）
- [GetHardwareBreakpointHitCount](#x64dbg-conditional_breakpoint_control-gethardwarebreakpointhitcount) — 获取硬件断点命中次数（hit count）。  （别名：GetHardwareBreakpointHitCount）
- [GetLibrarianBreakpointHitCount](#x64dbg-conditional_breakpoint_control-getlibrarianbreakpointhitcount) — 获取Librarian 断点命中次数（hit count）。  （别名：GetLibrarianBreakpointHitCount）
- [GetMemoryBreakpointHitCount](#x64dbg-conditional_breakpoint_control-getmemorybreakpointhitcount) — 获取内存断点命中次数（hit count）。  （别名：GetMemoryBreakpointHitCount）
- [ResetBreakpointHitCount](#x64dbg-conditional_breakpoint_control-resetbreakpointhitcount) — Conditional Breakpoint Control 分类命令：ResetBreakpointHitCount（请参考官方文档确认参数细节）。  （别名：ResetBreakpointHitCount）
- [ResetExceptionBreakpointHitCount](#x64dbg-conditional_breakpoint_control-resetexceptionbreakpointhitcount) — 重置异常断点命中次数（hit count 清零）。  （别名：ResetExceptionBreakpointHitCount）
- [ResetHardwareBreakpointHitCount](#x64dbg-conditional_breakpoint_control-resethardwarebreakpointhitcount) — 重置硬件断点命中次数（hit count 清零）。  （别名：ResetHardwareBreakpointHitCount）
- [ResetLibrarianBreakpointHitCount](#x64dbg-conditional_breakpoint_control-resetlibrarianbreakpointhitcount) — 重置Librarian 断点命中次数（hit count 清零）。  （别名：ResetLibrarianBreakpointHitCount）
- [ResetMemoryBreakpointHitCount](#x64dbg-conditional_breakpoint_control-resetmemorybreakpointhitcount) — 重置内存断点命中次数（hit count 清零）。  （别名：ResetMemoryBreakpointHitCount）
- [SetBreakpointCommand](#x64dbg-conditional_breakpoint_control-setbreakpointcommand) — 设置软件断点命中时自动执行的命令（可用于自动化）。  （别名：SetBreakpointCommand）
- [SetBreakpointCommandCondition](#x64dbg-conditional_breakpoint_control-setbreakpointcommandcondition) — 设置软件断点命中时执行命令的条件。  （别名：SetBreakpointCommandCondition）
- [SetBreakpointCondition](#x64dbg-conditional_breakpoint_control-setbreakpointcondition) — 设置软件断点的触发条件表达式（条件为真时才会命中）。  （别名：SetBreakpointCondition, bpcond, bpcnd）
- [SetBreakpointFastResume](#x64dbg-conditional_breakpoint_control-setbreakpointfastresume) — 设置软件断点“快速恢复/自动继续”选项（命中后自动继续）。  （别名：SetBreakpointFastResume）
- [SetBreakpointLog](#x64dbg-conditional_breakpoint_control-setbreakpointlog) — 设置软件断点命中时写入日志的内容/格式。  （别名：SetBreakpointLog, bplog, bpl）
- [SetBreakpointLogCondition](#x64dbg-conditional_breakpoint_control-setbreakpointlogcondition) — 设置软件断点的日志输出条件（条件为真时输出日志）。  （别名：SetBreakpointLogCondition, bplogcondition）
- [SetBreakpointName](#x64dbg-conditional_breakpoint_control-setbreakpointname) — 设置软件断点的名称/备注。  （别名：SetBreakpointName, bpname）
- [SetBreakpointSilent](#x64dbg-conditional_breakpoint_control-setbreakpointsilent) — 设置软件断点“静默”选项（命中时不弹出/不打断，可配合日志/命令）。  （别名：SetBreakpointSilent）
- [SetBreakpointSingleshoot](#x64dbg-conditional_breakpoint_control-setbreakpointsingleshoot) — 设置软件断点“单次命中”选项（命中一次后自动禁用/删除）。  （别名：SetBreakpointSingleshoot）
- [SetExceptionBreakpointCommand](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointcommand) — 设置异常断点命中时自动执行的命令。  （别名：SetExceptionBreakpointCommand）
- [SetExceptionBreakpointCommandCondition](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointcommandcondition) — 设置异常断点命中时执行命令的条件。  （别名：SetExceptionBreakpointCommandCondition）
- [SetExceptionBreakpointCondition](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointcondition) — 设置异常断点的触发条件表达式。  （别名：SetExceptionBreakpointCondition）
- [SetExceptionBreakpointFastResume](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointfastresume) — 设置异常断点“快速恢复/自动继续”选项。  （别名：SetExceptionBreakpointFastResume）
- [SetExceptionBreakpointLog](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointlog) — 设置异常断点命中时写入日志的内容/格式。  （别名：SetExceptionBreakpointLog）
- [SetExceptionBreakpointLogCondition](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointlogcondition) — 设置异常断点的日志输出条件。  （别名：SetExceptionBreakpointLogCondition）
- [SetExceptionBreakpointName](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointname) — 设置异常断点的名称/备注。  （别名：SetExceptionBreakpointName）
- [SetExceptionBreakpointSilent](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointsilent) — 设置异常断点“静默”选项。  （别名：SetExceptionBreakpointSilent）
- [SetExceptionBreakpointSingleshoot](#x64dbg-conditional_breakpoint_control-setexceptionbreakpointsingleshoot) — 设置异常断点“单次命中”选项。  （别名：SetExceptionBreakpointSingleshoot）
- [SetHardwareBreakpointCommand](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointcommand) — 设置硬件断点命中时自动执行的命令。  （别名：SetHardwareBreakpointCommand）
- [SetHardwareBreakpointCommandCondition](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointcommandcondition) — 设置硬件断点命中时执行命令的条件。  （别名：SetHardwareBreakpointCommandCondition）
- [SetHardwareBreakpointCondition](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointcondition) — 设置硬件断点的触发条件表达式。  （别名：SetHardwareBreakpointCondition, bphwcond）
- [SetHardwareBreakpointFastResume](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointfastresume) — 设置硬件断点“快速恢复/自动继续”选项。  （别名：SetHardwareBreakpointFastResume）
- [SetHardwareBreakpointLog](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointlog) — 设置硬件断点命中时写入日志的内容/格式。  （别名：SetHardwareBreakpointLog, bphwlog）
- [SetHardwareBreakpointLogCondition](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointlogcondition) — 设置硬件断点的日志输出条件。  （别名：SetHardwareBreakpointLogCondition, bphwlogcondition）
- [SetHardwareBreakpointName](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointname) — 设置硬件断点的名称/备注。  （别名：SetHardwareBreakpointName, bphwname）
- [SetHardwareBreakpointSilent](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointsilent) — 设置硬件断点“静默”选项。  （别名：SetHardwareBreakpointSilent）
- [SetHardwareBreakpointSingleshoot](#x64dbg-conditional_breakpoint_control-sethardwarebreakpointsingleshoot) — 设置硬件断点“单次命中”选项。  （别名：SetHardwareBreakpointSingleshoot）
- [SetLibrarianBreakpointCommand](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcommand) — 设置Librarian 断点命中时自动执行的命令。  （别名：SetLibrarianBreakpointCommand）
- [SetLibrarianBreakpointCommandCondition](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcommandcondition) — 设置Librarian 断点命中时执行命令的条件。  （别名：SetLibrarianBreakpointCommandCondition）
- [SetLibrarianBreakpointCondition](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcondition) — 设置Librarian 断点的触发条件表达式。  （别名：SetLibrarianBreakpointCondition）
- [SetLibrarianBreakpointFastResume](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointfastresume) — 设置Librarian 断点“快速恢复/自动继续”选项。  （别名：SetLibrarianBreakpointFastResume）
- [SetLibrarianBreakpointLog](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointlog) — 设置Librarian 断点命中时写入日志的内容/格式。  （别名：SetLibrarianBreakpointLog）
- [SetLibrarianBreakpointLogCondition](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointlogcondition) — 设置Librarian 断点的日志输出条件。  （别名：SetLibrarianBreakpointLogCondition）
- [SetLibrarianBreakpointName](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointname) — 设置Librarian 断点的名称/备注。  （别名：SetLibrarianBreakpointName）
- [SetLibrarianBreakpointSilent](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointsilent) — 设置Librarian 断点“静默”选项。  （别名：SetLibrarianBreakpointSilent）
- [SetLibrarianBreakpointSingleshoot](#x64dbg-conditional_breakpoint_control-setlibrarianbreakpointsingleshoot) — 设置Librarian 断点“单次命中”选项。  （别名：SetLibrarianBreakpointSingleshoot）
- [SetMemoryBreakpointCommand](#x64dbg-conditional_breakpoint_control-setmemorybreakpointcommand) — 设置内存断点命中时自动执行的命令。  （别名：SetMemoryBreakpointCommand）
- [SetMemoryBreakpointCommandCondition](#x64dbg-conditional_breakpoint_control-setmemorybreakpointcommandcondition) — 设置内存断点命中时执行命令的条件。  （别名：SetMemoryBreakpointCommandCondition）
- [SetMemoryBreakpointCondition](#x64dbg-conditional_breakpoint_control-setmemorybreakpointcondition) — 设置内存断点的触发条件表达式。  （别名：SetMemoryBreakpointCondition, bpmcond）
- [SetMemoryBreakpointFastResume](#x64dbg-conditional_breakpoint_control-setmemorybreakpointfastresume) — 设置内存断点“快速恢复/自动继续”选项。  （别名：SetMemoryBreakpointFastResume）
- [SetMemoryBreakpointLog](#x64dbg-conditional_breakpoint_control-setmemorybreakpointlog) — 设置内存断点命中时写入日志的内容/格式。  （别名：SetMemoryBreakpointLog, bpmlog）
- [SetMemoryBreakpointLogCondition](#x64dbg-conditional_breakpoint_control-setmemorybreakpointlogcondition) — 设置内存断点的日志输出条件。  （别名：SetMemoryBreakpointLogCondition, bpmlogcondition）
- [SetMemoryBreakpointName](#x64dbg-conditional_breakpoint_control-setmemorybreakpointname) — 设置内存断点的名称/备注。  （别名：SetMemoryBreakpointName, bpmname）
- [SetMemoryBreakpointSilent](#x64dbg-conditional_breakpoint_control-setmemorybreakpointsilent) — 设置内存断点“静默”选项。  （别名：SetMemoryBreakpointSilent）
- [SetMemoryBreakpointSingleshoot](#x64dbg-conditional_breakpoint_control-setmemorybreakpointsingleshoot) — 设置内存断点“单次命中”选项。  （别名：SetMemoryBreakpointSingleshoot）

### GetBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-getbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.getbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`GetBreakpointHitCount`
- **一句话用途**：Conditional Breakpoint Control 分类命令：GetBreakpointHitCount（请参考官方文档确认参数细节）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/getbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 读取断点命中次数，用于脚本决策（例如第 N 次才停）。
- 在长时间运行后统计命中分布，定位热点。
- 和 ResetHitCount 联动实现分段统计。

#### 语法（Syntax）

`GetBreakpointHitCount <args>`

#### 参数详解（Arguments）
- **`args`**（必需）：
  可变参数列表（逗号分隔）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- hitcount 类命令通常把计数写到 $result（或相关变量）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetBreakpointHitCount 1, 2, 3`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetExceptionBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-getexceptionbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.getexceptionbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`GetExceptionBreakpointHitCount`
- **一句话用途**：获取异常断点命中次数（hit count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/getexceptionbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 读取断点命中次数，用于脚本决策（例如第 N 次才停）。
- 在长时间运行后统计命中分布，定位热点。
- 和 ResetHitCount 联动实现分段统计。

#### 语法（Syntax）

`GetExceptionBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- hitcount 类命令通常把计数写到 $result（或相关变量）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetExceptionBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetHardwareBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-gethardwarebreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.gethardwarebreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`GetHardwareBreakpointHitCount`
- **一句话用途**：获取硬件断点命中次数（hit count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/gethardwarebreakpointhitcount.html`

#### 什么时候用（Use cases）
- 读取断点命中次数，用于脚本决策（例如第 N 次才停）。
- 在长时间运行后统计命中分布，定位热点。
- 和 ResetHitCount 联动实现分段统计。

#### 语法（Syntax）

`GetHardwareBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- hitcount 类命令通常把计数写到 $result（或相关变量）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetHardwareBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetLibrarianBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-getlibrarianbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.getlibrarianbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`GetLibrarianBreakpointHitCount`
- **一句话用途**：获取Librarian 断点命中次数（hit count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/getlibrarianbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 读取断点命中次数，用于脚本决策（例如第 N 次才停）。
- 在长时间运行后统计命中分布，定位热点。
- 和 ResetHitCount 联动实现分段统计。

#### 语法（Syntax）

`GetLibrarianBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- hitcount 类命令通常把计数写到 $result（或相关变量）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetLibrarianBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetMemoryBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-getmemorybreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.getmemorybreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`GetMemoryBreakpointHitCount`
- **一句话用途**：获取内存断点命中次数（hit count）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/getmemorybreakpointhitcount.html`

#### 什么时候用（Use cases）
- 读取断点命中次数，用于脚本决策（例如第 N 次才停）。
- 在长时间运行后统计命中分布，定位热点。
- 和 ResetHitCount 联动实现分段统计。

#### 语法（Syntax）

`GetMemoryBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- hitcount 类命令通常把计数写到 $result（或相关变量）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetMemoryBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ResetBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-resetbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.resetbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`ResetBreakpointHitCount`
- **一句话用途**：Conditional Breakpoint Control 分类命令：ResetBreakpointHitCount（请参考官方文档确认参数细节）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/resetbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 清零命中次数，重新开始统计。
- 在不同阶段对同一断点做分段计数。
- 与日志采样一起使用：每次采样后清零。

#### 语法（Syntax）

`ResetBreakpointHitCount <args>`

#### 参数详解（Arguments）
- **`args`**（必需）：
  可变参数列表（逗号分隔）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ResetBreakpointHitCount 1, 2, 3`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ResetExceptionBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-resetexceptionbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.resetexceptionbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`ResetExceptionBreakpointHitCount`
- **一句话用途**：重置异常断点命中次数（hit count 清零）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/resetexceptionbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 清零命中次数，重新开始统计。
- 在不同阶段对同一断点做分段计数。
- 与日志采样一起使用：每次采样后清零。

#### 语法（Syntax）

`ResetExceptionBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ResetExceptionBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ResetHardwareBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-resethardwarebreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.resethardwarebreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`ResetHardwareBreakpointHitCount`
- **一句话用途**：重置硬件断点命中次数（hit count 清零）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/resethardwarebreakpointhitcount.html`

#### 什么时候用（Use cases）
- 清零命中次数，重新开始统计。
- 在不同阶段对同一断点做分段计数。
- 与日志采样一起使用：每次采样后清零。

#### 语法（Syntax）

`ResetHardwareBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ResetHardwareBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ResetLibrarianBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-resetlibrarianbreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.resetlibrarianbreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`ResetLibrarianBreakpointHitCount`
- **一句话用途**：重置Librarian 断点命中次数（hit count 清零）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/resetlibrarianbreakpointhitcount.html`

#### 什么时候用（Use cases）
- 清零命中次数，重新开始统计。
- 在不同阶段对同一断点做分段计数。
- 与日志采样一起使用：每次采样后清零。

#### 语法（Syntax）

`ResetLibrarianBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ResetLibrarianBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ResetMemoryBreakpointHitCount  <a id="x64dbg-conditional_breakpoint_control-resetmemorybreakpointhitcount"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.resetmemorybreakpointhitcount`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`ResetMemoryBreakpointHitCount`
- **一句话用途**：重置内存断点命中次数（hit count 清零）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/resetmemorybreakpointhitcount.html`

#### 什么时候用（Use cases）
- 清零命中次数，重新开始统计。
- 在不同阶段对同一断点做分段计数。
- 与日志采样一起使用：每次采样后清零。

#### 语法（Syntax）

`ResetMemoryBreakpointHitCount <id_or_addr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ResetMemoryBreakpointHitCount 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointCommand  <a id="x64dbg-conditional_breakpoint_control-setbreakpointcommand"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointcommand`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetBreakpointCommand`
- **一句话用途**：设置软件断点命中时自动执行的命令（可用于自动化）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointcommand.html`

#### 什么时候用（Use cases）
- 断点命中时自动执行一串命令（例如 log、dump、run）。
- 实现“命中即补丁/跳过/修改寄存器”的自动化处理。
- 做半自动 fuzz：命中某点就记录并继续运行。

#### 语法（Syntax）

`SetBreakpointCommand <addr>, <command_string>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`command_string`**（必需）：
  要执行的命令字符串（x64dbg 命令行文本）。可用于断点命中时自动执行或脚本调用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointCommand 401000, "log \"hit\"; run"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointCommandCondition  <a id="x64dbg-conditional_breakpoint_control-setbreakpointcommandcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointcommandcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetBreakpointCommandCondition`
- **一句话用途**：设置软件断点命中时执行命令的条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointcommandcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetBreakpointCommandCondition <addr>, <cmd_condition_expr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`cmd_condition_expr`**（必需）：
  命令执行条件（布尔表达式）。表达式为真才执行对应 command。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointCommandCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointCondition  <a id="x64dbg-conditional_breakpoint_control-setbreakpointcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetBreakpointCondition`（别名：bpcond, bpcnd）
- **一句话用途**：设置软件断点的触发条件表达式（条件为真时才会命中）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetBreakpointCondition <addr>, <condition_expr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`condition_expr`**（必需）：
  触发条件表达式（布尔）。在断点/trace/脚本 IF 中常用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointCondition 401000, 1`
- `bpcnd 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointFastResume  <a id="x64dbg-conditional_breakpoint_control-setbreakpointfastresume"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointfastresume`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetBreakpointFastResume`
- **一句话用途**：设置软件断点“快速恢复/自动继续”选项（命中后自动继续）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointfastresume.html`

#### 什么时候用（Use cases）
- 让断点命中后自动继续（不打断运行），用于采样而非停下。
- 与 Log/Command 组合实现“无交互监控”。
- 避免在高频命中点阻塞调试会话。

#### 语法（Syntax）

`SetBreakpointFastResume <addr>, <0|1>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointFastResume 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointLog  <a id="x64dbg-conditional_breakpoint_control-setbreakpointlog"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointlog`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetBreakpointLog`（别名：bplog, bpl）
- **一句话用途**：设置软件断点命中时写入日志的内容/格式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointlog.html`

#### 什么时候用（Use cases）
- 断点命中时把关键寄存器/内存值输出到日志。
- 做“无停顿采样”（fastresume+log），实现运行态收集。
- 为后续离线分析生成结构化日志（配合固定格式模板）。

#### 语法（Syntax）

`SetBreakpointLog <addr>, <log_text_or_expr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`log_text_or_expr`**（必需）：
  日志内容：
  - 可以是固定文本；
  - 也可以是包含表达式占位符的模板（具体语法依实现）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointLog 401000, 1`
- `bpl 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointLogCondition  <a id="x64dbg-conditional_breakpoint_control-setbreakpointlogcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointlogcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetBreakpointLogCondition`（别名：bplogcondition）
- **一句话用途**：设置软件断点的日志输出条件（条件为真时输出日志）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointlogcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetBreakpointLogCondition <addr>, <log_condition_expr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`log_condition_expr`**（必需）：
  日志输出条件（布尔表达式）。用于控制是否输出日志。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointLogCondition 401000, 1`
- `bplogcondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointName  <a id="x64dbg-conditional_breakpoint_control-setbreakpointname"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointname`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetBreakpointName`（别名：bpname）
- **一句话用途**：设置软件断点的名称/备注。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointname.html`

#### 什么时候用（Use cases）
- 给断点加“可读的名字/备注”，便于 bplist/GUI 中识别。
- 脚本批量创建断点后统一命名，形成“断点清单”。
- 在导出 DB/截图时保留语义信息。

#### 语法（Syntax）

`SetBreakpointName <addr>, <name>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointName 401000, MyName`
- `bpname 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointSilent  <a id="x64dbg-conditional_breakpoint_control-setbreakpointsilent"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointsilent`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetBreakpointSilent`
- **一句话用途**：设置软件断点“静默”选项（命中时不弹出/不打断，可配合日志/命令）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointsilent.html`

#### 什么时候用（Use cases）
- 让断点静默（不弹窗、不打断 UI），更适合自动化采样。
- 配合 fastresume 形成完全后台化的断点动作。
- 减少对人工交互的依赖。

#### 语法（Syntax）

`SetBreakpointSilent <addr>, <0|1>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointSilent 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetBreakpointSingleshoot  <a id="x64dbg-conditional_breakpoint_control-setbreakpointsingleshoot"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setbreakpointsingleshoot`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetBreakpointSingleshoot`
- **一句话用途**：设置软件断点“单次命中”选项（命中一次后自动禁用/删除）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setbreakpointsingleshoot.html`

#### 什么时候用（Use cases）
- 一次性断点：只在第一次满足条件时触发，然后自动禁用/删除。
- 用于抓“首次初始化”或“首次解密”这种只发生一次的事件。
- 避免重复命中造成噪声或性能问题。

#### 语法（Syntax）

`SetBreakpointSingleshoot <addr>, <0|1>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetBreakpointSingleshoot 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointCommand  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointcommand"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointcommand`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointCommand`
- **一句话用途**：设置异常断点命中时自动执行的命令。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointcommand.html`

#### 什么时候用（Use cases）
- 断点命中时自动执行一串命令（例如 log、dump、run）。
- 实现“命中即补丁/跳过/修改寄存器”的自动化处理。
- 做半自动 fuzz：命中某点就记录并继续运行。

#### 语法（Syntax）

`SetExceptionBreakpointCommand <id_or_addr>, <command_string>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`command_string`**（必需）：
  要执行的命令字符串（x64dbg 命令行文本）。可用于断点命中时自动执行或脚本调用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointCommand 401000, "log \"hit\"; run"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointCommandCondition  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointcommandcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointcommandcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointCommandCondition`
- **一句话用途**：设置异常断点命中时执行命令的条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointcommandcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetExceptionBreakpointCommandCondition <id_or_addr>, <cmd_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`cmd_condition_expr`**（必需）：
  命令执行条件（布尔表达式）。表达式为真才执行对应 command。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointCommandCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointCondition  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointCondition`
- **一句话用途**：设置异常断点的触发条件表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetExceptionBreakpointCondition <id_or_addr>, <condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`condition_expr`**（必需）：
  触发条件表达式（布尔）。在断点/trace/脚本 IF 中常用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointFastResume  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointfastresume"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointfastresume`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointFastResume`
- **一句话用途**：设置异常断点“快速恢复/自动继续”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointfastresume.html`

#### 什么时候用（Use cases）
- 让断点命中后自动继续（不打断运行），用于采样而非停下。
- 与 Log/Command 组合实现“无交互监控”。
- 避免在高频命中点阻塞调试会话。

#### 语法（Syntax）

`SetExceptionBreakpointFastResume <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointFastResume 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointLog  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointlog"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointlog`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointLog`
- **一句话用途**：设置异常断点命中时写入日志的内容/格式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointlog.html`

#### 什么时候用（Use cases）
- 断点命中时把关键寄存器/内存值输出到日志。
- 做“无停顿采样”（fastresume+log），实现运行态收集。
- 为后续离线分析生成结构化日志（配合固定格式模板）。

#### 语法（Syntax）

`SetExceptionBreakpointLog <id_or_addr>, <log_text_or_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_text_or_expr`**（必需）：
  日志内容：
  - 可以是固定文本；
  - 也可以是包含表达式占位符的模板（具体语法依实现）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointLog 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointLogCondition  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointlogcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointlogcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointLogCondition`
- **一句话用途**：设置异常断点的日志输出条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointlogcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetExceptionBreakpointLogCondition <id_or_addr>, <log_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_condition_expr`**（必需）：
  日志输出条件（布尔表达式）。用于控制是否输出日志。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointLogCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointName  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointname"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointname`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointName`
- **一句话用途**：设置异常断点的名称/备注。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointname.html`

#### 什么时候用（Use cases）
- 给断点加“可读的名字/备注”，便于 bplist/GUI 中识别。
- 脚本批量创建断点后统一命名，形成“断点清单”。
- 在导出 DB/截图时保留语义信息。

#### 语法（Syntax）

`SetExceptionBreakpointName <id_or_addr>, <name>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointName 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointSilent  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointsilent"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointsilent`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointSilent`
- **一句话用途**：设置异常断点“静默”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointsilent.html`

#### 什么时候用（Use cases）
- 让断点静默（不弹窗、不打断 UI），更适合自动化采样。
- 配合 fastresume 形成完全后台化的断点动作。
- 减少对人工交互的依赖。

#### 语法（Syntax）

`SetExceptionBreakpointSilent <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointSilent 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetExceptionBreakpointSingleshoot  <a id="x64dbg-conditional_breakpoint_control-setexceptionbreakpointsingleshoot"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setexceptionbreakpointsingleshoot`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetExceptionBreakpointSingleshoot`
- **一句话用途**：设置异常断点“单次命中”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setexceptionbreakpointsingleshoot.html`

#### 什么时候用（Use cases）
- 一次性断点：只在第一次满足条件时触发，然后自动禁用/删除。
- 用于抓“首次初始化”或“首次解密”这种只发生一次的事件。
- 避免重复命中造成噪声或性能问题。

#### 语法（Syntax）

`SetExceptionBreakpointSingleshoot <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetExceptionBreakpointSingleshoot 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointCommand  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointcommand"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointcommand`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetHardwareBreakpointCommand`
- **一句话用途**：设置硬件断点命中时自动执行的命令。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointcommand.html`

#### 什么时候用（Use cases）
- 断点命中时自动执行一串命令（例如 log、dump、run）。
- 实现“命中即补丁/跳过/修改寄存器”的自动化处理。
- 做半自动 fuzz：命中某点就记录并继续运行。

#### 语法（Syntax）

`SetHardwareBreakpointCommand <id_or_addr>, <command_string>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`command_string`**（必需）：
  要执行的命令字符串（x64dbg 命令行文本）。可用于断点命中时自动执行或脚本调用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointCommand 401000, "log \"hit\"; run"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointCommandCondition  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointcommandcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointcommandcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetHardwareBreakpointCommandCondition`
- **一句话用途**：设置硬件断点命中时执行命令的条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointcommandcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetHardwareBreakpointCommandCondition <id_or_addr>, <cmd_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`cmd_condition_expr`**（必需）：
  命令执行条件（布尔表达式）。表达式为真才执行对应 command。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointCommandCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointCondition  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpointCondition`（别名：bphwcond）
- **一句话用途**：设置硬件断点的触发条件表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetHardwareBreakpointCondition <id_or_addr>, <condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`condition_expr`**（必需）：
  触发条件表达式（布尔）。在断点/trace/脚本 IF 中常用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointCondition 401000, 1`
- `bphwcond 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointFastResume  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointfastresume"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointfastresume`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetHardwareBreakpointFastResume`
- **一句话用途**：设置硬件断点“快速恢复/自动继续”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointfastresume.html`

#### 什么时候用（Use cases）
- 让断点命中后自动继续（不打断运行），用于采样而非停下。
- 与 Log/Command 组合实现“无交互监控”。
- 避免在高频命中点阻塞调试会话。

#### 语法（Syntax）

`SetHardwareBreakpointFastResume <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointFastResume 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointLog  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointlog"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointlog`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpointLog`（别名：bphwlog）
- **一句话用途**：设置硬件断点命中时写入日志的内容/格式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointlog.html`

#### 什么时候用（Use cases）
- 断点命中时把关键寄存器/内存值输出到日志。
- 做“无停顿采样”（fastresume+log），实现运行态收集。
- 为后续离线分析生成结构化日志（配合固定格式模板）。

#### 语法（Syntax）

`SetHardwareBreakpointLog <id_or_addr>, <log_text_or_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_text_or_expr`**（必需）：
  日志内容：
  - 可以是固定文本；
  - 也可以是包含表达式占位符的模板（具体语法依实现）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointLog 401000, 1`
- `bphwlog 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointLogCondition  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointlogcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointlogcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpointLogCondition`（别名：bphwlogcondition）
- **一句话用途**：设置硬件断点的日志输出条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointlogcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetHardwareBreakpointLogCondition <id_or_addr>, <log_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_condition_expr`**（必需）：
  日志输出条件（布尔表达式）。用于控制是否输出日志。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointLogCondition 401000, 1`
- `bphwlogcondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointName  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointname"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointname`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetHardwareBreakpointName`（别名：bphwname）
- **一句话用途**：设置硬件断点的名称/备注。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointname.html`

#### 什么时候用（Use cases）
- 给断点加“可读的名字/备注”，便于 bplist/GUI 中识别。
- 脚本批量创建断点后统一命名，形成“断点清单”。
- 在导出 DB/截图时保留语义信息。

#### 语法（Syntax）

`SetHardwareBreakpointName <id_or_addr>, <name>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointName 401000, MyName`
- `bphwname 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointSilent  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointsilent"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointsilent`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetHardwareBreakpointSilent`
- **一句话用途**：设置硬件断点“静默”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointsilent.html`

#### 什么时候用（Use cases）
- 让断点静默（不弹窗、不打断 UI），更适合自动化采样。
- 配合 fastresume 形成完全后台化的断点动作。
- 减少对人工交互的依赖。

#### 语法（Syntax）

`SetHardwareBreakpointSilent <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointSilent 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetHardwareBreakpointSingleshoot  <a id="x64dbg-conditional_breakpoint_control-sethardwarebreakpointsingleshoot"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.sethardwarebreakpointsingleshoot`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetHardwareBreakpointSingleshoot`
- **一句话用途**：设置硬件断点“单次命中”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/sethardwarebreakpointsingleshoot.html`

#### 什么时候用（Use cases）
- 一次性断点：只在第一次满足条件时触发，然后自动禁用/删除。
- 用于抓“首次初始化”或“首次解密”这种只发生一次的事件。
- 避免重复命中造成噪声或性能问题。

#### 语法（Syntax）

`SetHardwareBreakpointSingleshoot <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetHardwareBreakpointSingleshoot 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointCommand  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcommand"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointcommand`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointCommand`
- **一句话用途**：设置Librarian 断点命中时自动执行的命令。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointcommand.html`

#### 什么时候用（Use cases）
- 断点命中时自动执行一串命令（例如 log、dump、run）。
- 实现“命中即补丁/跳过/修改寄存器”的自动化处理。
- 做半自动 fuzz：命中某点就记录并继续运行。

#### 语法（Syntax）

`SetLibrarianBreakpointCommand <id_or_addr>, <command_string>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`command_string`**（必需）：
  要执行的命令字符串（x64dbg 命令行文本）。可用于断点命中时自动执行或脚本调用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointCommand 401000, "log \"hit\"; run"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointCommandCondition  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcommandcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointcommandcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointCommandCondition`
- **一句话用途**：设置Librarian 断点命中时执行命令的条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointcommandcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetLibrarianBreakpointCommandCondition <id_or_addr>, <cmd_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`cmd_condition_expr`**（必需）：
  命令执行条件（布尔表达式）。表达式为真才执行对应 command。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointCommandCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointCondition  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointCondition`
- **一句话用途**：设置Librarian 断点的触发条件表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetLibrarianBreakpointCondition <id_or_addr>, <condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`condition_expr`**（必需）：
  触发条件表达式（布尔）。在断点/trace/脚本 IF 中常用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointFastResume  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointfastresume"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointfastresume`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointFastResume`
- **一句话用途**：设置Librarian 断点“快速恢复/自动继续”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointfastresume.html`

#### 什么时候用（Use cases）
- 让断点命中后自动继续（不打断运行），用于采样而非停下。
- 与 Log/Command 组合实现“无交互监控”。
- 避免在高频命中点阻塞调试会话。

#### 语法（Syntax）

`SetLibrarianBreakpointFastResume <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointFastResume 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointLog  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointlog"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointlog`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointLog`
- **一句话用途**：设置Librarian 断点命中时写入日志的内容/格式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointlog.html`

#### 什么时候用（Use cases）
- 断点命中时把关键寄存器/内存值输出到日志。
- 做“无停顿采样”（fastresume+log），实现运行态收集。
- 为后续离线分析生成结构化日志（配合固定格式模板）。

#### 语法（Syntax）

`SetLibrarianBreakpointLog <id_or_addr>, <log_text_or_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_text_or_expr`**（必需）：
  日志内容：
  - 可以是固定文本；
  - 也可以是包含表达式占位符的模板（具体语法依实现）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointLog 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointLogCondition  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointlogcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointlogcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointLogCondition`
- **一句话用途**：设置Librarian 断点的日志输出条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointlogcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetLibrarianBreakpointLogCondition <id_or_addr>, <log_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_condition_expr`**（必需）：
  日志输出条件（布尔表达式）。用于控制是否输出日志。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointLogCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointName  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointname"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointname`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointName`
- **一句话用途**：设置Librarian 断点的名称/备注。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointname.html`

#### 什么时候用（Use cases）
- 给断点加“可读的名字/备注”，便于 bplist/GUI 中识别。
- 脚本批量创建断点后统一命名，形成“断点清单”。
- 在导出 DB/截图时保留语义信息。

#### 语法（Syntax）

`SetLibrarianBreakpointName <id_or_addr>, <name>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointName 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointSilent  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointsilent"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointsilent`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointSilent`
- **一句话用途**：设置Librarian 断点“静默”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointsilent.html`

#### 什么时候用（Use cases）
- 让断点静默（不弹窗、不打断 UI），更适合自动化采样。
- 配合 fastresume 形成完全后台化的断点动作。
- 减少对人工交互的依赖。

#### 语法（Syntax）

`SetLibrarianBreakpointSilent <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointSilent 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetLibrarianBreakpointSingleshoot  <a id="x64dbg-conditional_breakpoint_control-setlibrarianbreakpointsingleshoot"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setlibrarianbreakpointsingleshoot`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetLibrarianBreakpointSingleshoot`
- **一句话用途**：设置Librarian 断点“单次命中”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setlibrarianbreakpointsingleshoot.html`

#### 什么时候用（Use cases）
- 一次性断点：只在第一次满足条件时触发，然后自动禁用/删除。
- 用于抓“首次初始化”或“首次解密”这种只发生一次的事件。
- 避免重复命中造成噪声或性能问题。

#### 语法（Syntax）

`SetLibrarianBreakpointSingleshoot <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetLibrarianBreakpointSingleshoot 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointCommand  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointcommand"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointcommand`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetMemoryBreakpointCommand`
- **一句话用途**：设置内存断点命中时自动执行的命令。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointcommand.html`

#### 什么时候用（Use cases）
- 断点命中时自动执行一串命令（例如 log、dump、run）。
- 实现“命中即补丁/跳过/修改寄存器”的自动化处理。
- 做半自动 fuzz：命中某点就记录并继续运行。

#### 语法（Syntax）

`SetMemoryBreakpointCommand <id_or_addr>, <command_string>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`command_string`**（必需）：
  要执行的命令字符串（x64dbg 命令行文本）。可用于断点命中时自动执行或脚本调用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointCommand 401000, "log \"hit\"; run"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointCommandCondition  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointcommandcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointcommandcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetMemoryBreakpointCommandCondition`
- **一句话用途**：设置内存断点命中时执行命令的条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointcommandcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetMemoryBreakpointCommandCondition <id_or_addr>, <cmd_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`cmd_condition_expr`**（必需）：
  命令执行条件（布尔表达式）。表达式为真才执行对应 command。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointCommandCondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointCondition  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpointCondition`（别名：bpmcond）
- **一句话用途**：设置内存断点的触发条件表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetMemoryBreakpointCondition <id_or_addr>, <condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`condition_expr`**（必需）：
  触发条件表达式（布尔）。在断点/trace/脚本 IF 中常用。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointCondition 401000, 1`
- `bpmcond 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointFastResume  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointfastresume"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointfastresume`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetMemoryBreakpointFastResume`
- **一句话用途**：设置内存断点“快速恢复/自动继续”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointfastresume.html`

#### 什么时候用（Use cases）
- 让断点命中后自动继续（不打断运行），用于采样而非停下。
- 与 Log/Command 组合实现“无交互监控”。
- 避免在高频命中点阻塞调试会话。

#### 语法（Syntax）

`SetMemoryBreakpointFastResume <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointFastResume 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointLog  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointlog"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointlog`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpointLog`（别名：bpmlog）
- **一句话用途**：设置内存断点命中时写入日志的内容/格式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointlog.html`

#### 什么时候用（Use cases）
- 断点命中时把关键寄存器/内存值输出到日志。
- 做“无停顿采样”（fastresume+log），实现运行态收集。
- 为后续离线分析生成结构化日志（配合固定格式模板）。

#### 语法（Syntax）

`SetMemoryBreakpointLog <id_or_addr>, <log_text_or_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_text_or_expr`**（必需）：
  日志内容：
  - 可以是固定文本；
  - 也可以是包含表达式占位符的模板（具体语法依实现）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointLog 401000, 1`
- `bpmlog 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointLogCondition  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointlogcondition"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointlogcondition`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpointLogCondition`（别名：bpmlogcondition）
- **一句话用途**：设置内存断点的日志输出条件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointlogcondition.html`

#### 什么时候用（Use cases）
- 把断点变成“条件断点”，只有满足条件才真正停下/触发动作。
- 用寄存器/内存字段过滤噪声命中（例如只在 rax==0 时）。
- 配合 hitcount 实现“第 N 次命中才停”。

#### 语法（Syntax）

`SetMemoryBreakpointLogCondition <id_or_addr>, <log_condition_expr>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`log_condition_expr`**（必需）：
  日志输出条件（布尔表达式）。用于控制是否输出日志。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointLogCondition 401000, 1`
- `bpmlogcondition 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointName  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointname"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointname`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令/别名**：`SetMemoryBreakpointName`（别名：bpmname）
- **一句话用途**：设置内存断点的名称/备注。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointname.html`

#### 什么时候用（Use cases）
- 给断点加“可读的名字/备注”，便于 bplist/GUI 中识别。
- 脚本批量创建断点后统一命名，形成“断点清单”。
- 在导出 DB/截图时保留语义信息。

#### 语法（Syntax）

`SetMemoryBreakpointName <id_or_addr>, <name>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointName 401000, MyName`
- `bpmname 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointSilent  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointsilent"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointsilent`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetMemoryBreakpointSilent`
- **一句话用途**：设置内存断点“静默”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointsilent.html`

#### 什么时候用（Use cases）
- 让断点静默（不弹窗、不打断 UI），更适合自动化采样。
- 配合 fastresume 形成完全后台化的断点动作。
- 减少对人工交互的依赖。

#### 语法（Syntax）

`SetMemoryBreakpointSilent <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointSilent 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetMemoryBreakpointSingleshoot  <a id="x64dbg-conditional_breakpoint_control-setmemorybreakpointsingleshoot"></a>

- **Skill ID**：`x64dbg.conditional_breakpoint_control.setmemorybreakpointsingleshoot`
- **分类**：条件断点/断点属性 (Conditional Breakpoint Control)
- **命令**：`SetMemoryBreakpointSingleshoot`
- **一句话用途**：设置内存断点“单次命中”选项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/setmemorybreakpointsingleshoot.html`

#### 什么时候用（Use cases）
- 一次性断点：只在第一次满足条件时触发，然后自动禁用/删除。
- 用于抓“首次初始化”或“首次解密”这种只发生一次的事件。
- 避免重复命中造成噪声或性能问题。

#### 语法（Syntax）

`SetMemoryBreakpointSingleshoot <id_or_addr>, <0|1>`

#### 参数详解（Arguments）
- **`id_or_addr`**（必需）：
  ID 或地址：
  - ID/索引（来自列表）；
  - 或 VA。
  建议优先 VA，ID 适合对 GUI 列表做批量操作。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetMemoryBreakpointSingleshoot 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 指令跟踪 (Tracing)

Tracing 命令用于执行“指令级跟踪”，包括：
- Trace Into/Over（带条件版本）；
- Trace 命中时的日志/命令/条件；
- Trace 记录控制（开始/停止记录）。

Trace 很容易产生大量数据与性能开销：建议设置合理的条件、上限与过滤策略。

### 本分类命令索引

- [RunToParty](#x64dbg-tracing-runtoparty) — Tracing 分类命令：RunToParty（请参考官方文档确认参数细节）。  （别名：RunToParty）
- [RunToUserCode](#x64dbg-tracing-runtousercode) — Tracing 分类命令：RunToUserCode（请参考官方文档确认参数细节）。  （别名：RunToUserCode, rtu）
- [StartTraceRecording](#x64dbg-tracing-starttracerecording) — 开始运行轨迹记录（trace recording）。  （别名：StartTraceRecording, StartRunTrace, opentrace）
- [StopTraceRecording](#x64dbg-tracing-stoptracerecording) — 停止运行轨迹记录。  （别名：StopTraceRecording, StopRunTrace, tc）
- [TraceIntoBeyondTraceCoverage](#x64dbg-tracing-traceintobeyondtracecoverage) — 按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。  （别名：TraceIntoBeyondTraceCoverage, TraceIntoBeyondTraceRecord, tibt）
- [TraceIntoConditional](#x64dbg-tracing-traceintoconditional) — 按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。  （别名：TraceIntoConditional, ticnd）
- [TraceIntoIntoTraceCoverage](#x64dbg-tracing-traceintointotracecoverage) — 按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。  （别名：TraceIntoIntoTraceCoverage, TraceIntoIntoTraceRecord, tiit）
- [TraceOverBeyondTraceCoverage](#x64dbg-tracing-traceoverbeyondtracecoverage) — 按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。  （别名：TraceOverBeyondTraceCoverage, TraceOverBeyondTraceRecord, tobt）
- [TraceOverConditional](#x64dbg-tracing-traceoverconditional) — 按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。  （别名：TraceOverConditional, tocnd）
- [TraceOverIntoTraceCoverage](#x64dbg-tracing-traceoverintotracecoverage) — 按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。  （别名：TraceOverIntoTraceCoverage, TraceOverIntoTraceRecord, toit）
- [TraceSetCommand](#x64dbg-tracing-tracesetcommand) — 设置运行轨迹记录命中时执行的命令模板。  （别名：TraceSetCommand, SetTraceCommand）
- [TraceSetLog](#x64dbg-tracing-tracesetlog) — 设置运行轨迹记录命中时的日志输出模板。  （别名：TraceSetLog, SetTraceLog）
- [TraceSetLogFile](#x64dbg-tracing-tracesetlogfile) — 设置运行轨迹记录的日志文件路径。  （别名：TraceSetLogFile, SetTraceLogFile）

### RunToParty  <a id="x64dbg-tracing-runtoparty"></a>

- **Skill ID**：`x64dbg.tracing.runtoparty`
- **分类**：指令跟踪 (Tracing)
- **命令**：`RunToParty`
- **一句话用途**：Tracing 分类命令：RunToParty（请参考官方文档确认参数细节）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/runtoparty.html`

#### 什么时候用（Use cases）
- 指令跟踪与记录控制。
- 在自动化中抓取执行路径。
- 与日志/引用视图联动。

#### 语法（Syntax）

`RunToParty <args>`

#### 参数详解（Arguments）
- **`args`**（必需）：
  可变参数列表（逗号分隔）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `RunToParty 1, 2, 3`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### RunToUserCode  <a id="x64dbg-tracing-runtousercode"></a>

- **Skill ID**：`x64dbg.tracing.runtousercode`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`RunToUserCode`（别名：rtu）
- **一句话用途**：Tracing 分类命令：RunToUserCode（请参考官方文档确认参数细节）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/runtousercode.html`

#### 什么时候用（Use cases）
- 指令跟踪与记录控制。
- 在自动化中抓取执行路径。
- 与日志/引用视图联动。

#### 语法（Syntax）

`RunToUserCode <args>`

#### 参数详解（Arguments）
- **`args`**（必需）：
  可变参数列表（逗号分隔）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `RunToUserCode 1, 2, 3`
- `rtu 1, 2, 3`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StartTraceRecording  <a id="x64dbg-tracing-starttracerecording"></a>

- **Skill ID**：`x64dbg.tracing.starttracerecording`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`StartTraceRecording`（别名：StartRunTrace, opentrace）
- **一句话用途**：开始运行轨迹记录（trace recording）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/starttracerecording.html`

#### 什么时候用（Use cases）
- 开始/停止 trace 记录，把一段执行过程存入 trace 缓冲/文件。
- 与 trace 条件命令配合实现“触发后开始录制，结束点停止”。
- 用于生成可复现的执行证据（供后续离线分析）。

#### 语法（Syntax）

`StartTraceRecording [<options>]`

#### 参数详解（Arguments）
- **`options`**（可选）：
  可选项集合（通常为逗号分隔的标志位或 key=value 对）。具体可用项依命令实现而定。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `StartTraceRecording "1"`
- `opentrace "1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StopTraceRecording  <a id="x64dbg-tracing-stoptracerecording"></a>

- **Skill ID**：`x64dbg.tracing.stoptracerecording`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`StopTraceRecording`（别名：StopRunTrace, tc）
- **一句话用途**：停止运行轨迹记录。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/stoptracerecording.html`

#### 什么时候用（Use cases）
- 开始/停止 trace 记录，把一段执行过程存入 trace 缓冲/文件。
- 与 trace 条件命令配合实现“触发后开始录制，结束点停止”。
- 用于生成可复现的执行证据（供后续离线分析）。

#### 语法（Syntax）

`StopTraceRecording`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `StopTraceRecording`
- `tc`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceIntoBeyondTraceCoverage  <a id="x64dbg-tracing-traceintobeyondtracecoverage"></a>

- **Skill ID**：`x64dbg.tracing.traceintobeyondtracecoverage`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceIntoBeyondTraceCoverage`（别名：TraceIntoBeyondTraceRecord, tibt）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceintobeyondtracecoverage.html`

#### 什么时候用（Use cases）
- 跟踪执行流，进入函数调用内部（Trace Into）。
- 在循环/解密等关键区域记录每条指令的行为。
- 配合条件限制 trace 范围，避免爆量。

#### 语法（Syntax）

`TraceIntoBeyondTraceCoverage <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceIntoBeyondTraceCoverage "rax==0"`
- `tibt "rax==0"`
- `TraceIntoBeyondTraceCoverage "rax==0", 1`
- `tibt "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceIntoConditional  <a id="x64dbg-tracing-traceintoconditional"></a>

- **Skill ID**：`x64dbg.tracing.traceintoconditional`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceIntoConditional`（别名：ticnd）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceintoconditional.html`

#### 什么时候用（Use cases）
- 跟踪执行流，进入函数调用内部（Trace Into）。
- 在循环/解密等关键区域记录每条指令的行为。
- 配合条件限制 trace 范围，避免爆量。

#### 语法（Syntax）

`TraceIntoConditional <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceIntoConditional "rax==0"`
- `ticnd "rax==0"`
- `TraceIntoConditional "rax==0", 1`
- `ticnd "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceIntoIntoTraceCoverage  <a id="x64dbg-tracing-traceintointotracecoverage"></a>

- **Skill ID**：`x64dbg.tracing.traceintointotracecoverage`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceIntoIntoTraceCoverage`（别名：TraceIntoIntoTraceRecord, tiit）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Into（逐指令追踪并进入调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceintointotracecoverage.html`

#### 什么时候用（Use cases）
- 跟踪执行流，进入函数调用内部（Trace Into）。
- 在循环/解密等关键区域记录每条指令的行为。
- 配合条件限制 trace 范围，避免爆量。

#### 语法（Syntax）

`TraceIntoIntoTraceCoverage <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceIntoIntoTraceCoverage "rax==0"`
- `tiit "rax==0"`
- `TraceIntoIntoTraceCoverage "rax==0", 1`
- `tiit "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceOverBeyondTraceCoverage  <a id="x64dbg-tracing-traceoverbeyondtracecoverage"></a>

- **Skill ID**：`x64dbg.tracing.traceoverbeyondtracecoverage`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceOverBeyondTraceCoverage`（别名：TraceOverBeyondTraceRecord, tobt）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceoverbeyondtracecoverage.html`

#### 什么时候用（Use cases）
- 跟踪执行流，但对 call 做 step over（Trace Over）。
- 想记录主流程但不想展开库函数时使用。
- 在高层逻辑分析时减少 trace 深度。

#### 语法（Syntax）

`TraceOverBeyondTraceCoverage <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceOverBeyondTraceCoverage "rax==0"`
- `tobt "rax==0"`
- `TraceOverBeyondTraceCoverage "rax==0", 1`
- `tobt "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceOverConditional  <a id="x64dbg-tracing-traceoverconditional"></a>

- **Skill ID**：`x64dbg.tracing.traceoverconditional`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceOverConditional`（别名：tocnd）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceoverconditional.html`

#### 什么时候用（Use cases）
- 跟踪执行流，但对 call 做 step over（Trace Over）。
- 想记录主流程但不想展开库函数时使用。
- 在高层逻辑分析时减少 trace 深度。

#### 语法（Syntax）

`TraceOverConditional <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceOverConditional "rax==0"`
- `tocnd "rax==0"`
- `TraceOverConditional "rax==0", 1`
- `tocnd "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceOverIntoTraceCoverage  <a id="x64dbg-tracing-traceoverintotracecoverage"></a>

- **Skill ID**：`x64dbg.tracing.traceoverintotracecoverage`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceOverIntoTraceCoverage`（别名：TraceOverIntoTraceRecord, toit）
- **一句话用途**：按条件/覆盖率等规则进行 Trace Over（逐指令追踪但跳过调用）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/traceoverintotracecoverage.html`

#### 什么时候用（Use cases）
- 跟踪执行流，但对 call 做 step over（Trace Over）。
- 想记录主流程但不想展开库函数时使用。
- 在高层逻辑分析时减少 trace 深度。

#### 语法（Syntax）

`TraceOverIntoTraceCoverage <condition_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`condition_or_range`**（必需）：
  条件表达式或范围参数。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceOverIntoTraceCoverage "rax==0"`
- `toit "rax==0"`
- `TraceOverIntoTraceCoverage "rax==0", 1`
- `toit "rax==0", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceSetCommand  <a id="x64dbg-tracing-tracesetcommand"></a>

- **Skill ID**：`x64dbg.tracing.tracesetcommand`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceSetCommand`（别名：SetTraceCommand）
- **一句话用途**：设置运行轨迹记录命中时执行的命令模板。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/tracesetcommand.html`

#### 什么时候用（Use cases）
- 指令跟踪与记录控制。
- 在自动化中抓取执行路径。
- 与日志/引用视图联动。

#### 语法（Syntax）

`TraceSetCommand <command_template>`

#### 参数详解（Arguments）
- **`command_template`**（必需）：
  命令模板（带占位符的 command 文本），用于 trace/断点自动命令等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceSetCommand "log \"trace\""`
- `SetTraceCommand "log \"trace\""`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceSetLog  <a id="x64dbg-tracing-tracesetlog"></a>

- **Skill ID**：`x64dbg.tracing.tracesetlog`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceSetLog`（别名：SetTraceLog）
- **一句话用途**：设置运行轨迹记录命中时的日志输出模板。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/tracesetlog.html`

#### 什么时候用（Use cases）
- 指令跟踪与记录控制。
- 在自动化中抓取执行路径。
- 与日志/引用视图联动。

#### 语法（Syntax）

`TraceSetLog <log_template>`

#### 参数详解（Arguments）
- **`log_template`**（必需）：
  日志模板（带占位符）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceSetLog "hit at {cip}"`
- `SetTraceLog "hit at {cip}"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### TraceSetLogFile  <a id="x64dbg-tracing-tracesetlogfile"></a>

- **Skill ID**：`x64dbg.tracing.tracesetlogfile`
- **分类**：指令跟踪 (Tracing)
- **命令/别名**：`TraceSetLogFile`（别名：SetTraceLogFile）
- **一句话用途**：设置运行轨迹记录的日志文件路径。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/tracing/tracesetlogfile.html`

#### 什么时候用（Use cases）
- 指令跟踪与记录控制。
- 在自动化中抓取执行路径。
- 与日志/引用视图联动。

#### 语法（Syntax）

`TraceSetLogFile <path>`

#### 参数详解（Arguments）
- **`path`**（必需）：
  路径参数。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `TraceSetLogFile "C:\\temp"`
- `SetTraceLogFile "C:\\temp"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 线程控制 (Thread Control)

线程控制命令用于：
- 创建线程、切换当前线程上下文；
- 暂停/恢复线程（单个或全部）；
- 结束线程（kill）；
- 设置线程优先级、线程名。

自动化注意：
- 很多线程操作要求调试器处于暂停状态，否则结果不可预期；
- 杀线程可能破坏被调试进程的逻辑（甚至导致崩溃），应尽量先 suspend/resume/切换再决定是否 kill。

### 本分类命令索引

- [createthread](#x64dbg-thread_control-createthread) — 在被调试进程中创建新线程。  （别名：createthread, threadcreate, newthread, threadnew）
- [killthread](#x64dbg-thread_control-killthread) — 结束/杀死指定线程。  （别名：killthread, threadkill）
- [resumeallthreads](#x64dbg-thread_control-resumeallthreads) — 恢复所有线程。  （别名：resumeallthreads, threadresumeall）
- [resumethread](#x64dbg-thread_control-resumethread) — 恢复指定线程。  （别名：resumethread, threadresume）
- [setthreadname](#x64dbg-thread_control-setthreadname) — 设置线程名称（用于 UI/调试标识）。  （别名：setthreadname, threadsetname）
- [setthreadpriority](#x64dbg-thread_control-setthreadpriority) — 设置线程优先级。  （别名：setthreadpriority, setprioritythread, threadsetpriority）
- [suspendallthreads](#x64dbg-thread_control-suspendallthreads) — 挂起所有线程。  （别名：suspendallthreads, threadsuspendall）
- [suspendthread](#x64dbg-thread_control-suspendthread) — 挂起指定线程。  （别名：suspendthread, threadsuspend）
- [switchthread](#x64dbg-thread_control-switchthread) — 切换当前调试线程。  （别名：switchthread, threadswitch）

### createthread  <a id="x64dbg-thread_control-createthread"></a>

- **Skill ID**：`x64dbg.thread_control.createthread`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`createthread`（别名：threadcreate, newthread, threadnew）
- **一句话用途**：在被调试进程中创建新线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/createthread.html`

#### 什么时候用（Use cases）
- 在目标进程中创建新线程（用于测试/注入式场景）。
- 复现某些线程入口逻辑或触发特定回调。
- 与 suspend/resume 组合控制并发时序。

#### 语法（Syntax）

`createthread <start_addr>[, <param>]`

#### 参数详解（Arguments）
- **`start_addr`**（必需）：
  范围起始地址（VA）。
- **`param`**（可选）：
  参数（上下文相关）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `createthread 401000`
- `newthread 401000`
- `createthread 401000, 1`
- `newthread 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### killthread  <a id="x64dbg-thread_control-killthread"></a>

- **Skill ID**：`x64dbg.thread_control.killthread`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`killthread`（别名：threadkill）
- **一句话用途**：结束/杀死指定线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/killthread.html`

#### 什么时候用（Use cases）
- 终止异常线程（例如死循环线程）以继续分析其他部分。
- 在受控环境下快速验证“去掉某线程”对程序行为的影响。
- 作为最后手段的干预（高风险）。

#### 语法（Syntax）

`killthread <thread_id>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 强制关闭句柄/结束线程可能导致目标进程不稳定或立即崩溃。

#### 示例（Examples）
- `killthread 1234`
- `threadkill 1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### resumeallthreads  <a id="x64dbg-thread_control-resumeallthreads"></a>

- **Skill ID**：`x64dbg.thread_control.resumeallthreads`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`resumeallthreads`（别名：threadresumeall）
- **一句话用途**：恢复所有线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/resumeallthreads.html`

#### 什么时候用（Use cases）
- 冻结/恢复线程以控制竞态（race condition）与时间窗口。
- 在多线程程序里隔离噪声线程，聚焦目标线程。
- 结合 watch/trace 在暂停的同时保持其他线程稳定。

#### 语法（Syntax）

`resumeallthreads`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `resumeallthreads`
- `threadresumeall`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### resumethread  <a id="x64dbg-thread_control-resumethread"></a>

- **Skill ID**：`x64dbg.thread_control.resumethread`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`resumethread`（别名：threadresume）
- **一句话用途**：恢复指定线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/resumethread.html`

#### 什么时候用（Use cases）
- 冻结/恢复线程以控制竞态（race condition）与时间窗口。
- 在多线程程序里隔离噪声线程，聚焦目标线程。
- 结合 watch/trace 在暂停的同时保持其他线程稳定。

#### 语法（Syntax）

`resumethread <thread_id>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `resumethread 1234`
- `threadresume 1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setthreadname  <a id="x64dbg-thread_control-setthreadname"></a>

- **Skill ID**：`x64dbg.thread_control.setthreadname`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`setthreadname`（别名：threadsetname）
- **一句话用途**：设置线程名称（用于 UI/调试标识）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/setthreadname.html`

#### 什么时候用（Use cases）
- 给线程设置易读名称，便于在 GUI/日志中识别。
- 脚本批量命名线程（按入口/模块/用途分类）。
- 结合 switchthread 形成“按名切换/采样”的自动化流程。

#### 语法（Syntax）

`setthreadname <thread_id>, <name>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setthreadname 1234, MyName`
- `threadsetname 1234, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setthreadpriority  <a id="x64dbg-thread_control-setthreadpriority"></a>

- **Skill ID**：`x64dbg.thread_control.setthreadpriority`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`setthreadpriority`（别名：setprioritythread, threadsetpriority）
- **一句话用途**：设置线程优先级。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/setthreadpriority.html`

#### 什么时候用（Use cases）
- 调整线程优先级以改变调度行为（复现/规避竞态）。
- 让目标线程更容易获得 CPU 以加快触发关键路径。
- 在调试卡顿时降低某些线程优先级。

#### 语法（Syntax）

`setthreadpriority <thread_id>, <priority>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。
- **`priority`**（必需）：
  线程优先级（枚举或数值）。例如 idle/normal/highest/timecritical 或对应数值。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setthreadpriority 1234, 1`
- `setprioritythread 1234, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### suspendallthreads  <a id="x64dbg-thread_control-suspendallthreads"></a>

- **Skill ID**：`x64dbg.thread_control.suspendallthreads`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`suspendallthreads`（别名：threadsuspendall）
- **一句话用途**：挂起所有线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/suspendallthreads.html`

#### 什么时候用（Use cases）
- 冻结/恢复线程以控制竞态（race condition）与时间窗口。
- 在多线程程序里隔离噪声线程，聚焦目标线程。
- 结合 watch/trace 在暂停的同时保持其他线程稳定。

#### 语法（Syntax）

`suspendallthreads`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `suspendallthreads`
- `threadsuspendall`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### suspendthread  <a id="x64dbg-thread_control-suspendthread"></a>

- **Skill ID**：`x64dbg.thread_control.suspendthread`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`suspendthread`（别名：threadsuspend）
- **一句话用途**：挂起指定线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/suspendthread.html`

#### 什么时候用（Use cases）
- 冻结/恢复线程以控制竞态（race condition）与时间窗口。
- 在多线程程序里隔离噪声线程，聚焦目标线程。
- 结合 watch/trace 在暂停的同时保持其他线程稳定。

#### 语法（Syntax）

`suspendthread <thread_id>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `suspendthread 1234`
- `threadsuspend 1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### switchthread  <a id="x64dbg-thread_control-switchthread"></a>

- **Skill ID**：`x64dbg.thread_control.switchthread`
- **分类**：线程控制 (Thread Control)
- **命令/别名**：`switchthread`（别名：threadswitch）
- **一句话用途**：切换当前调试线程。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/thread-control/switchthread.html`

#### 什么时候用（Use cases）
- 切换调试器的当前线程上下文（查看不同线程的寄存器/栈/执行点）。
- 在断点命中后定位真正触发的线程。
- 为线程级断点/观察建立正确上下文。

#### 语法（Syntax）

`switchthread <thread_id>`

#### 参数详解（Arguments）
- **`thread_id`**（必需）：
  线程 ID（TID）。注意与线程句柄/索引区分。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `switchthread 1234`
- `threadswitch 1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 内存操作 (Memory Operations)

内存操作命令覆盖：
- 申请/释放内存（alloc/free）；
- 填充/拷贝（memset/memcpy）；
- 查询/设置页权限（get/set page rights）；
- 保存数据到文件（savedata）、生成 minidump。

自动化时务必区分：
- **被调试进程内存** vs **调试器自身**；
- **VA** vs **文件偏移**；
- 是否需要暂停态、是否需要页面可写。

### 本分类命令索引

- [alloc](#x64dbg-memory_operations-alloc) — 在被调试进程中申请一段内存。  （别名：alloc）
- [Fill](#x64dbg-memory_operations-fill) — 用指定字节值填充一段内存（memset）。  （别名：Fill, memset）
- [free](#x64dbg-memory_operations-free) — 释放先前 alloc 的内存。  （别名：free）
- [getpagerights](#x64dbg-memory_operations-getpagerights) — 查询内存页保护属性（page rights）。  （别名：getpagerights, getrightspage）
- [memcpy](#x64dbg-memory_operations-memcpy) — 在被调试进程内复制内存（memcpy）。  （别名：memcpy）
- [minidump](#x64dbg-memory_operations-minidump) — 生成（mini）dump 文件。  （别名：minidump）
- [savedata](#x64dbg-memory_operations-savedata) — 将内存数据保存到文件。  （别名：savedata）
- [setpagerights](#x64dbg-memory_operations-setpagerights) — 设置内存页保护属性（page rights）。  （别名：setpagerights, setrightspage）

### alloc  <a id="x64dbg-memory_operations-alloc"></a>

- **Skill ID**：`x64dbg.memory_operations.alloc`
- **分类**：内存操作 (Memory Operations)
- **命令**：`alloc`
- **一句话用途**：在被调试进程中申请一段内存。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/alloc.html`

#### 什么时候用（Use cases）
- 在目标进程中申请/释放内存块（用于注入、构造缓冲区、测试 patch）。
- 为写入 shellcode/字符串/结构体准备空间（alloc）。
- 清理临时分配，减少对目标进程的长期影响（free）。

#### 语法（Syntax）

`alloc <size>[, <rights>]`

#### 参数详解（Arguments）
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。
- **`rights`**（可选）：
  通用参数（请结合官方页面确认具体格式）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `alloc 10`
- `alloc 10, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### Fill  <a id="x64dbg-memory_operations-fill"></a>

- **Skill ID**：`x64dbg.memory_operations.fill`
- **分类**：内存操作 (Memory Operations)
- **命令/别名**：`Fill`（别名：memset）
- **一句话用途**：用指定字节值填充一段内存（memset）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/fill.html`

#### 什么时候用（Use cases）
- 用固定字节填充一段内存（清零、写 0xCC、写 NOP 等）。
- 初始化结构体/数组缓冲区。
- 在 patch 前先把区域清理干净。

#### 语法（Syntax）

`Fill <addr>, <size>, <byte>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。
- **`byte`**（必需）：
  单字节值（0~255）。用于填充等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `Fill 401000, 10, 90`
- `memset 401000, 10, 90`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### free  <a id="x64dbg-memory_operations-free"></a>

- **Skill ID**：`x64dbg.memory_operations.free`
- **分类**：内存操作 (Memory Operations)
- **命令**：`free`
- **一句话用途**：释放先前 alloc 的内存。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/free.html`

#### 什么时候用（Use cases）
- 在目标进程中申请/释放内存块（用于注入、构造缓冲区、测试 patch）。
- 为写入 shellcode/字符串/结构体准备空间（alloc）。
- 清理临时分配，减少对目标进程的长期影响（free）。

#### 语法（Syntax）

`free <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `free 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### getpagerights  <a id="x64dbg-memory_operations-getpagerights"></a>

- **Skill ID**：`x64dbg.memory_operations.getpagerights`
- **分类**：内存操作 (Memory Operations)
- **命令/别名**：`getpagerights`（别名：getrightspage）
- **一句话用途**：查询内存页保护属性（page rights）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/getpagerights.html`

#### 什么时候用（Use cases）
- 查询或修改页面权限（R/W/X）。
- 在写代码段前临时把页面改成可写，写完再恢复。
- 排查访问违例：确认某地址页权限是否符合预期。

#### 语法（Syntax）

`getpagerights <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `getpagerights 401000`
- `getrightspage 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### memcpy  <a id="x64dbg-memory_operations-memcpy"></a>

- **Skill ID**：`x64dbg.memory_operations.memcpy`
- **分类**：内存操作 (Memory Operations)
- **命令**：`memcpy`
- **一句话用途**：在被调试进程内复制内存（memcpy）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/memcpy.html`

#### 什么时候用（Use cases）
- 把内存从 src 拷贝到 dest（构造/搬运缓冲区）。
- 在 patch 时复制一段代码/数据到新位置（配合 alloc）。
- 做快照：把关键区域复制到备份区后再修改。

#### 语法（Syntax）

`memcpy <dest>, <src>, <size>`

#### 参数详解（Arguments）
- **`dest`**（必需）：
  可写入的目标（assignable operand）。一般可以是：
  - 寄存器（例如 rax/eax/rcx… 或 x64dbg 的别名寄存器）；
  - 变量（由 var 命令创建的用户变量）；
  - 内存操作数（例如 [addr]、[reg+offset] 这类解引用表达式，前提是该页可写）。
  注意：dest 必须是“可赋值”的对象；如果传入常量或不可写表达式会失败。
- **`src`**（必需）：
  源值表达式（read-only）。可以是常量、寄存器、变量、内存解引用结果、表达式计算结果等。
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `memcpy rax, 1, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### minidump  <a id="x64dbg-memory_operations-minidump"></a>

- **Skill ID**：`x64dbg.memory_operations.minidump`
- **分类**：内存操作 (Memory Operations)
- **命令**：`minidump`
- **一句话用途**：生成（mini）dump 文件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/minidump.html`

#### 什么时候用（Use cases）
- 生成 minidump 以便在外部工具中分析崩溃/现场。
- 在异常断点触发时自动保存现场。
- 用于把复杂现场分享给团队做离线复现。

#### 语法（Syntax）

`minidump <file_path>`

#### 参数详解（Arguments）
- **`file_path`**（必需）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- 需要可写的输出路径（或确保当前目录 chd 已设置）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `minidump "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### savedata  <a id="x64dbg-memory_operations-savedata"></a>

- **Skill ID**：`x64dbg.memory_operations.savedata`
- **分类**：内存操作 (Memory Operations)
- **命令**：`savedata`
- **一句话用途**：将内存数据保存到文件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/savedata.html`

#### 什么时候用（Use cases）
- 把目标进程内存中的数据导出到文件（dump）。
- 在命中断点时自动导出关键缓冲区做离线分析。
- 用于提取解密后的内容、配置、资源等。

#### 语法（Syntax）

`savedata <addr>, <size>, <file_path>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。
- **`file_path`**（必需）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- 需要可写的输出路径（或确保当前目录 chd 已设置）。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `savedata 401000, 10, "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setpagerights  <a id="x64dbg-memory_operations-setpagerights"></a>

- **Skill ID**：`x64dbg.memory_operations.setpagerights`
- **分类**：内存操作 (Memory Operations)
- **命令/别名**：`setpagerights`（别名：setrightspage）
- **一句话用途**：设置内存页保护属性（page rights）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/memory-operations/setpagerights.html`

#### 什么时候用（Use cases）
- 查询或修改页面权限（R/W/X）。
- 在写代码段前临时把页面改成可写，写完再恢复。
- 排查访问违例：确认某地址页权限是否符合预期。

#### 语法（Syntax）

`setpagerights <addr>, <rights>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`rights`**（必需）：
  通用参数（请结合官方页面确认具体格式）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。
- 修改页权限可能影响代码执行（尤其把代码页改成可写/不可执行）。

#### 示例（Examples）
- `setpagerights 401000, 1`
- `setrightspage 401000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 操作系统控制 (Operating System Control)

操作系统控制命令主要涉及 Windows 权限（Privilege）与句柄（Handle）：
- Enable/DisablePrivilege、GetPrivilegeState；
- 关闭句柄 handleclose。

这类命令通常用于提升调试器/目标进程的操作权限（例如需要 SeDebugPrivilege），或在特定逆向场景里处理句柄资源。

### 本分类命令索引

- [DisablePrivilege](#x64dbg-operating_system_control-disableprivilege) — 禁用指定权限。  （别名：DisablePrivilege）
- [EnablePrivilege](#x64dbg-operating_system_control-enableprivilege) — 启用指定权限（Windows privilege）。  （别名：EnablePrivilege）
- [GetPrivilegeState](#x64dbg-operating_system_control-getprivilegestate) — 查询指定权限当前是否启用。  （别名：GetPrivilegeState）
- [handleclose](#x64dbg-operating_system_control-handleclose) — 关闭指定句柄（handle）。  （别名：handleclose, closehandle）

### DisablePrivilege  <a id="x64dbg-operating_system_control-disableprivilege"></a>

- **Skill ID**：`x64dbg.operating_system_control.disableprivilege`
- **分类**：操作系统控制 (Operating System Control)
- **命令**：`DisablePrivilege`
- **一句话用途**：禁用指定权限。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/operating-system-control/disableprivilege.html`

#### 什么时候用（Use cases）
- 启用/禁用某个 Windows privilege（常见 SeDebugPrivilege）。
- 在附加高权限进程前确保权限已启用。
- 在脚本里检测并自动修正权限状态。

#### 语法（Syntax）

`DisablePrivilege <priv_name>`

#### 参数详解（Arguments）
- **`priv_name`**（必需）：
  Windows 权限名（Privilege），例如 SeDebugPrivilege。大小写通常不敏感，但建议按标准拼写。

#### 执行上下文 / 前置条件（Preconditions）
- 需要以足够权限运行调试器以调整进程 token privilege。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisablePrivilege SeDebugPrivilege`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnablePrivilege  <a id="x64dbg-operating_system_control-enableprivilege"></a>

- **Skill ID**：`x64dbg.operating_system_control.enableprivilege`
- **分类**：操作系统控制 (Operating System Control)
- **命令**：`EnablePrivilege`
- **一句话用途**：启用指定权限（Windows privilege）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/operating-system-control/enableprivilege.html`

#### 什么时候用（Use cases）
- 启用/禁用某个 Windows privilege（常见 SeDebugPrivilege）。
- 在附加高权限进程前确保权限已启用。
- 在脚本里检测并自动修正权限状态。

#### 语法（Syntax）

`EnablePrivilege <priv_name>`

#### 参数详解（Arguments）
- **`priv_name`**（必需）：
  Windows 权限名（Privilege），例如 SeDebugPrivilege。大小写通常不敏感，但建议按标准拼写。

#### 执行上下文 / 前置条件（Preconditions）
- 需要以足够权限运行调试器以调整进程 token privilege。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnablePrivilege SeDebugPrivilege`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetPrivilegeState  <a id="x64dbg-operating_system_control-getprivilegestate"></a>

- **Skill ID**：`x64dbg.operating_system_control.getprivilegestate`
- **分类**：操作系统控制 (Operating System Control)
- **命令**：`GetPrivilegeState`
- **一句话用途**：查询指定权限当前是否启用。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/operating-system-control/getprivilegestate.html`

#### 什么时候用（Use cases）
- 启用/禁用某个 Windows privilege（常见 SeDebugPrivilege）。
- 在附加高权限进程前确保权限已启用。
- 在脚本里检测并自动修正权限状态。

#### 语法（Syntax）

`GetPrivilegeState <priv_name>`

#### 参数详解（Arguments）
- **`priv_name`**（必需）：
  Windows 权限名（Privilege），例如 SeDebugPrivilege。大小写通常不敏感，但建议按标准拼写。

#### 执行上下文 / 前置条件（Preconditions）
- 需要以足够权限运行调试器以调整进程 token privilege。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 读取类命令可能会把读取到的值写到 $result 或返回字符串到特定变量（依实现）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetPrivilegeState SeDebugPrivilege`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### handleclose  <a id="x64dbg-operating_system_control-handleclose"></a>

- **Skill ID**：`x64dbg.operating_system_control.handleclose`
- **分类**：操作系统控制 (Operating System Control)
- **命令/别名**：`handleclose`（别名：closehandle）
- **一句话用途**：关闭指定句柄（handle）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/operating-system-control/handleclose.html`

#### 什么时候用（Use cases）
- 关闭指定句柄（在调试场景里模拟 CloseHandle）。
- 释放目标进程持有的某些资源，观察行为变化。
- 排查句柄泄漏/锁资源问题时作为实验手段（高风险）。

#### 语法（Syntax）

`handleclose <handle>`

#### 参数详解（Arguments）
- **`handle`**（必需）：
  Windows HANDLE 值（通常是整数/十六进制）。注意区分句柄值与地址。

#### 执行上下文 / 前置条件（Preconditions）
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 强制关闭句柄/结束线程可能导致目标进程不稳定或立即崩溃。

#### 示例（Examples）
- `handleclose 0x1234`
- `closehandle 0x1234`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 监视/Watch 控制 (Watch Control)

Watch/Watchdog 命令用于建立“观察点”：
- 监视某表达式/内存值随时间变化；
- 可设置触发条件、名称、注释；
- watchdog 常用于自动检测“值变化”并做提示/动作。

与内存断点不同，watch 更偏“轮询/评估表达式”，开销模式不一样。

### 本分类命令索引

- [AddWatch](#x64dbg-watch_control-addwatch) — 在 Watch 视图中新增一条监视表达式。  （别名：AddWatch）
- [CheckWatchdog](#x64dbg-watch_control-checkwatchdog) — 检查 Watchdog 状态（是否触发）。  （别名：CheckWatchdog）
- [DelWatch](#x64dbg-watch_control-delwatch) — 删除一条 Watch。  （别名：DelWatch）
- [SetWatchdog](#x64dbg-watch_control-setwatchdog) — 设置 Watchdog（监视变化/触发）。  （别名：SetWatchdog）
- [SetWatchExpression](#x64dbg-watch_control-setwatchexpression) — 修改某条 Watch 的表达式。  （别名：SetWatchExpression）
- [SetWatchName](#x64dbg-watch_control-setwatchname) — 设置某条 Watch 的名称。  （别名：SetWatchName）

### AddWatch  <a id="x64dbg-watch_control-addwatch"></a>

- **Skill ID**：`x64dbg.watch_control.addwatch`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`AddWatch`
- **一句话用途**：在 Watch 视图中新增一条监视表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/addwatch.html`

#### 什么时候用（Use cases）
- 添加一个 watch 表达式，持续观察其值。
- 观察某地址内存字段随运行变化（不设置内存断点）。
- 在自动化中把关键状态输出到 watchdog 结果中。

#### 语法（Syntax）

`AddWatch <expression>`

#### 参数详解（Arguments）
- **`expression`**（必需）：
  表达式文本（x64dbg expression）。通常用于 watch/condition 等需要“稍后再求值”的场景。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddWatch "rax==0"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### CheckWatchdog  <a id="x64dbg-watch_control-checkwatchdog"></a>

- **Skill ID**：`x64dbg.watch_control.checkwatchdog`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`CheckWatchdog`
- **一句话用途**：检查 Watchdog 状态（是否触发）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/checkwatchdog.html`

#### 什么时候用（Use cases）
- 启用/配置 watchdog 自动检查机制。
- 当表达式变化/满足条件时触发提示或动作。
- 用作轻量级监控（相比 trace/断点）。

#### 语法（Syntax）

`CheckWatchdog`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `CheckWatchdog`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DelWatch  <a id="x64dbg-watch_control-delwatch"></a>

- **Skill ID**：`x64dbg.watch_control.delwatch`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`DelWatch`
- **一句话用途**：删除一条 Watch。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/delwatch.html`

#### 什么时候用（Use cases）
- 移除不再需要的 watch，减少评估开销。
- 自动化脚本结束时清理 watch 列表。
- 当表达式错误导致大量错误提示时快速删除。

#### 语法（Syntax）

`DelWatch <index_or_name>`

#### 参数详解（Arguments）
- **`index_or_name`**（必需）：
  索引或名称。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DelWatch 0`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetWatchdog  <a id="x64dbg-watch_control-setwatchdog"></a>

- **Skill ID**：`x64dbg.watch_control.setwatchdog`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`SetWatchdog`
- **一句话用途**：设置 Watchdog（监视变化/触发）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/setwatchdog.html`

#### 什么时候用（Use cases）
- 启用/配置 watchdog 自动检查机制。
- 当表达式变化/满足条件时触发提示或动作。
- 用作轻量级监控（相比 trace/断点）。

#### 语法（Syntax）

`SetWatchdog <index>, <0|1>`

#### 参数详解（Arguments）
- **`index`**（必需）：
  索引（一般从 0 或 1 开始，取决于列表）。建议先用 list/get 命令确认。
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetWatchdog 0, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetWatchExpression  <a id="x64dbg-watch_control-setwatchexpression"></a>

- **Skill ID**：`x64dbg.watch_control.setwatchexpression`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`SetWatchExpression`
- **一句话用途**：修改某条 Watch 的表达式。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/setwatchexpression.html`

#### 什么时候用（Use cases）
- 更新 watch 的计算表达式。
- 在同一个 watch 名称下动态切换监控目标。
- 把复杂表达式抽象成可读 watch。

#### 语法（Syntax）

`SetWatchExpression <index>, <expression>`

#### 参数详解（Arguments）
- **`index`**（必需）：
  索引（一般从 0 或 1 开始，取决于列表）。建议先用 list/get 命令确认。
- **`expression`**（必需）：
  表达式文本（x64dbg expression）。通常用于 watch/condition 等需要“稍后再求值”的场景。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetWatchExpression 0, "rax==0"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SetWatchName  <a id="x64dbg-watch_control-setwatchname"></a>

- **Skill ID**：`x64dbg.watch_control.setwatchname`
- **分类**：监视/Watch 控制 (Watch Control)
- **命令**：`SetWatchName`
- **一句话用途**：设置某条 Watch 的名称。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/watch-control/setwatchname.html`

#### 什么时候用（Use cases）
- 给 watch 命名并加注释，便于识别。
- 脚本批量创建 watch 后统一标注来源。
- 在长时间调试会话中保持可读性。

#### 语法（Syntax）

`SetWatchName <index>, <name>`

#### 参数详解（Arguments）
- **`index`**（必需）：
  索引（一般从 0 或 1 开始，取决于列表）。建议先用 list/get 命令确认。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SetWatchName 0, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 变量 (Variables)

变量命令用于维护脚本/命令行层的用户变量：
- var 创建/赋值；
- vardel 删除；
- varlist 列表。

建议 Agent 把复杂中间结果存到变量中，便于后续命令复用与调试脚本可读性。

### 本分类命令索引

- [var](#x64dbg-variables-var) — 创建/定义一个用户变量。  （别名：var, varnew）
- [vardel](#x64dbg-variables-vardel) — 删除一个用户变量。  （别名：vardel）
- [varlist](#x64dbg-variables-varlist) — 列出当前所有用户变量。  （别名：varlist）

### var  <a id="x64dbg-variables-var"></a>

- **Skill ID**：`x64dbg.variables.var`
- **分类**：变量 (Variables)
- **命令/别名**：`var`（别名：varnew）
- **一句话用途**：创建/定义一个用户变量。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/variables/var.html`

#### 什么时候用（Use cases）
- 创建或更新一个用户变量（保存中间值、地址、计数器）。
- 把复杂表达式结果缓存，避免重复计算。
- 在脚本多步骤之间传递状态。

#### 语法（Syntax）

`var <name>, <expression>`

#### 参数详解（Arguments）
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。
- **`expression`**（必需）：
  表达式文本（x64dbg expression）。通常用于 watch/condition 等需要“稍后再求值”的场景。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `var MyName, "rax==0"`
- `varnew MyName, "rax==0"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### vardel  <a id="x64dbg-variables-vardel"></a>

- **Skill ID**：`x64dbg.variables.vardel`
- **分类**：变量 (Variables)
- **命令**：`vardel`
- **一句话用途**：删除一个用户变量。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/variables/vardel.html`

#### 什么时候用（Use cases）
- 删除变量，释放命名空间，避免污染后续脚本。
- 在脚本结束时做清理。
- 当变量被误用时快速移除。

#### 语法（Syntax）

`vardel <name>`

#### 参数详解（Arguments）
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `vardel MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### varlist  <a id="x64dbg-variables-varlist"></a>

- **Skill ID**：`x64dbg.variables.varlist`
- **分类**：变量 (Variables)
- **命令**：`varlist`
- **一句话用途**：列出当前所有用户变量。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/variables/varlist.html`

#### 什么时候用（Use cases）
- 列出当前所有变量，方便调试脚本状态。
- 自动化中把变量快照写入日志。
- 排查变量名冲突/覆盖问题。

#### 语法（Syntax）

`varlist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `varlist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 搜索 (Searching)

搜索命令用于：
- 在内存/模块中查找字节模式/字符串；
- 搜索汇编指令序列（findasm）；
- GUID 查找；
- 引用查找（ref find / ref range / string refs）；
- module call 查找；
- 设置最大搜索结果数。

搜索通常会写入“引用视图/结果列表”，适合与 GUI 命令（refinit/refadd/refget）联动。

### 本分类命令索引

- [find](#x64dbg-searching-find) — 在内存/模块中搜索数据（首个匹配）。  （别名：find）
- [findall](#x64dbg-searching-findall) — 搜索全部匹配并输出列表。  （别名：findall）
- [findallmem](#x64dbg-searching-findallmem) — 在所有内存页中搜索全部匹配。  （别名：findallmem, findmemall）
- [findasm](#x64dbg-searching-findasm) — 按汇编指令模式搜索。  （别名：findasm, asmfind）
- [findguid](#x64dbg-searching-findguid) — 搜索 GUID。  （别名：findguid, guidfind）
- [modcallfind](#x64dbg-searching-modcallfind) — 查找对某模块导出/调用的引用（module call find）。  （别名：modcallfind）
- [reffind](#x64dbg-searching-reffind) — 查找对某地址/符号的引用（references）。  （别名：reffind, findref, ref）
- [reffindrange](#x64dbg-searching-reffindrange) — 按范围查找交叉引用（references）结果。  （别名：reffindrange, findrefrange, refrange）
- [refstr](#x64dbg-searching-refstr) — 查找对字符串的引用（string references）。  （别名：refstr, strref）
- [setmaxfindresult](#x64dbg-searching-setmaxfindresult) — 设置 find/findall 等的最大结果数量。  （别名：setmaxfindresult, findsetmaxresult）

### find  <a id="x64dbg-searching-find"></a>

- **Skill ID**：`x64dbg.searching.find`
- **分类**：搜索 (Searching)
- **命令**：`find`
- **一句话用途**：在内存/模块中搜索数据（首个匹配）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/find.html`

#### 什么时候用（Use cases）
- 在指定范围/模块中查找字节/字符串/汇编模式。
- 定位常量、字符串引用、特征码（sig）等。
- 在脚本里实现自动定位，然后下断点或注释。

#### 语法（Syntax）

`find <pattern>[, <range>]`

#### 参数详解（Arguments）
- **`pattern`**（必需）：
  搜索模式（字节模式/字符串/通配符）。具体格式取决于命令。
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `find "90 90 90"`
- `find "90 90 90", 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### findall  <a id="x64dbg-searching-findall"></a>

- **Skill ID**：`x64dbg.searching.findall`
- **分类**：搜索 (Searching)
- **命令**：`findall`
- **一句话用途**：搜索全部匹配并输出列表。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/findall.html`

#### 什么时候用（Use cases）
- 在指定范围/模块中查找字节/字符串/汇编模式。
- 定位常量、字符串引用、特征码（sig）等。
- 在脚本里实现自动定位，然后下断点或注释。

#### 语法（Syntax）

`findall <pattern>[, <range>]`

#### 参数详解（Arguments）
- **`pattern`**（必需）：
  搜索模式（字节模式/字符串/通配符）。具体格式取决于命令。
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `findall "90 90 90"`
- `findall "90 90 90", 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### findallmem  <a id="x64dbg-searching-findallmem"></a>

- **Skill ID**：`x64dbg.searching.findallmem`
- **分类**：搜索 (Searching)
- **命令/别名**：`findallmem`（别名：findmemall）
- **一句话用途**：在所有内存页中搜索全部匹配。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/findallmem.html`

#### 什么时候用（Use cases）
- 在指定范围/模块中查找字节/字符串/汇编模式。
- 定位常量、字符串引用、特征码（sig）等。
- 在脚本里实现自动定位，然后下断点或注释。

#### 语法（Syntax）

`findallmem <pattern>`

#### 参数详解（Arguments）
- **`pattern`**（必需）：
  搜索模式（字节模式/字符串/通配符）。具体格式取决于命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `findallmem "90 90 90"`
- `findmemall "90 90 90"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### findasm  <a id="x64dbg-searching-findasm"></a>

- **Skill ID**：`x64dbg.searching.findasm`
- **分类**：搜索 (Searching)
- **命令/别名**：`findasm`（别名：asmfind）
- **一句话用途**：按汇编指令模式搜索。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/findasm.html`

#### 什么时候用（Use cases）
- 在指定范围/模块中查找字节/字符串/汇编模式。
- 定位常量、字符串引用、特征码（sig）等。
- 在脚本里实现自动定位，然后下断点或注释。

#### 语法（Syntax）

`findasm <asm_pattern>[, <range>]`

#### 参数详解（Arguments）
- **`asm_pattern`**（必需）：
  汇编模式/匹配表达式，用于搜索特定指令序列。
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `findasm "push*;mov*;call*"`
- `asmfind "push*;mov*;call*"`
- `findasm "push*;mov*;call*", 401000, 402000`
- `asmfind "push*;mov*;call*", 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### findguid  <a id="x64dbg-searching-findguid"></a>

- **Skill ID**：`x64dbg.searching.findguid`
- **分类**：搜索 (Searching)
- **命令/别名**：`findguid`（别名：guidfind）
- **一句话用途**：搜索 GUID。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/findguid.html`

#### 什么时候用（Use cases）
- 在指定范围/模块中查找字节/字符串/汇编模式。
- 定位常量、字符串引用、特征码（sig）等。
- 在脚本里实现自动定位，然后下断点或注释。

#### 语法（Syntax）

`findguid <guid>[, <range>]`

#### 参数详解（Arguments）
- **`guid`**（必需）：
  GUID（例如 {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}）。用于 findguid。
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `findguid "{01234567-89AB-CDEF-0123-456789ABCDEF}"`
- `guidfind "{01234567-89AB-CDEF-0123-456789ABCDEF}"`
- `findguid "{01234567-89AB-CDEF-0123-456789ABCDEF}", 401000, 402000`
- `guidfind "{01234567-89AB-CDEF-0123-456789ABCDEF}", 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### modcallfind  <a id="x64dbg-searching-modcallfind"></a>

- **Skill ID**：`x64dbg.searching.modcallfind`
- **分类**：搜索 (Searching)
- **命令**：`modcallfind`
- **一句话用途**：查找对某模块导出/调用的引用（module call find）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/modcallfind.html`

#### 什么时候用（Use cases）
- 在模块中查找 call 指令/调用点（用于 API 调用分析）。
- 快速枚举某模块对外部函数的调用位置。
- 与分析/注释联动实现自动标注。

#### 语法（Syntax）

`modcallfind <module>[, <func>]`

#### 参数详解（Arguments）
- **`module`**（必需）：
  模块名（如 user32.dll）。
- **`func`**（可选）：
  通用参数（请结合官方页面确认具体格式）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `modcallfind kernel32.dll`
- `modcallfind kernel32.dll, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### reffind  <a id="x64dbg-searching-reffind"></a>

- **Skill ID**：`x64dbg.searching.reffind`
- **分类**：搜索 (Searching)
- **命令/别名**：`reffind`（别名：findref, ref）
- **一句话用途**：查找对某地址/符号的引用（references）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/reffind.html`

#### 什么时候用（Use cases）
- 查找引用（xrefs）：谁引用了某地址/字符串。
- 在逆向中定位调用点、读写点。
- 配合 refinit/refadd/refget 将结果写入引用视图。

#### 语法（Syntax）

`reffind <addr_or_symbol>`

#### 参数详解（Arguments）
- **`addr_or_symbol`**（必需）：
  地址或符号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `reffind kernel32.GetProcAddress`
- `ref kernel32.GetProcAddress`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### reffindrange  <a id="x64dbg-searching-reffindrange"></a>

- **Skill ID**：`x64dbg.searching.reffindrange`
- **分类**：搜索 (Searching)
- **命令/别名**：`reffindrange`（别名：findrefrange, refrange）
- **一句话用途**：按范围查找交叉引用（references）结果。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/reffindrange.html`

#### 什么时候用（Use cases）
- 查找引用（xrefs）：谁引用了某地址/字符串。
- 在逆向中定位调用点、读写点。
- 配合 refinit/refadd/refget 将结果写入引用视图。

#### 语法（Syntax）

`reffindrange <start>, <end>`

#### 参数详解（Arguments）
- **`start`**（必需）：
  起始地址或起始索引（上下文相关）。
- **`end`**（必需）：
  结束地址或结束索引（上下文相关）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `reffindrange 401000, 402000`
- `refrange 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### refstr  <a id="x64dbg-searching-refstr"></a>

- **Skill ID**：`x64dbg.searching.refstr`
- **分类**：搜索 (Searching)
- **命令/别名**：`refstr`（别名：strref）
- **一句话用途**：查找对字符串的引用（string references）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/refstr.html`

#### 什么时候用（Use cases）
- 查找引用（xrefs）：谁引用了某地址/字符串。
- 在逆向中定位调用点、读写点。
- 配合 refinit/refadd/refget 将结果写入引用视图。

#### 语法（Syntax）

`refstr <string_or_addr>`

#### 参数详解（Arguments）
- **`string_or_addr`**（必需）：
  字符串或地址（指向字符串的地址）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `refstr "Hello"`
- `strref "Hello"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setmaxfindresult  <a id="x64dbg-searching-setmaxfindresult"></a>

- **Skill ID**：`x64dbg.searching.setmaxfindresult`
- **分类**：搜索 (Searching)
- **命令/别名**：`setmaxfindresult`（别名：findsetmaxresult）
- **一句话用途**：设置 find/findall 等的最大结果数量。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/searching/setmaxfindresult.html`

#### 什么时候用（Use cases）
- 设置搜索结果上限，避免一次 findall 产生过多结果导致卡顿。
- 在自动化批处理时控制内存与 GUI 负载。
- 对大模块扫描前先设置合理上限。

#### 语法（Syntax）

`setmaxfindresult <n>`

#### 参数详解（Arguments）
- **`n`**（必需）：
  数量/上限等整数参数。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 搜索类命令通常还会在 GUI 的引用视图/结果列表中产生条目；必要时用 refget 读取。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setmaxfindresult 100`
- `findsetmaxresult 100`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 用户数据库/标注持久化 (User Database)

用户数据库（User DB）用于持久化你的逆向标注：
- 注释、标签、书签；
- 函数边界、函数参数；
- 保存/加载/清空数据库等。

自动化常见用法：
- 批量导入注释/标签；
- 在脚本里根据分析结果写回 DB；
- 生成可复现的分析状态。

### 本分类命令索引

- [argumentadd](#x64dbg-user_database-argumentadd) — 添加函数参数信息（argument）。  （别名：argumentadd）
- [argumentclear](#x64dbg-user_database-argumentclear) — 清空函数参数信息。  （别名：argumentclear）
- [argumentdel](#x64dbg-user_database-argumentdel) — 删除函数参数信息。  （别名：argumentdel）
- [argumentlist](#x64dbg-user_database-argumentlist) — 列出所有函数参数信息。  （别名：argumentlist）
- [bookmarkclear](#x64dbg-user_database-bookmarkclear) — 清空所有书签。  （别名：bookmarkclear）
- [bookmarkdel](#x64dbg-user_database-bookmarkdel) — 删除书签。  （别名：bookmarkdel, bookmarkc）
- [bookmarklist](#x64dbg-user_database-bookmarklist) — 列出所有书签。  （别名：bookmarklist）
- [bookmarkset](#x64dbg-user_database-bookmarkset) — 设置书签（bookmark）。  （别名：bookmarkset, bookmark）
- [commentclear](#x64dbg-user_database-commentclear) — 清空所有注释。  （别名：commentclear）
- [commentdel](#x64dbg-user_database-commentdel) — 删除指定地址的注释。  （别名：commentdel, cmtc, cmtdel）
- [commentlist](#x64dbg-user_database-commentlist) — 列出所有注释。  （别名：commentlist）
- [commentset](#x64dbg-user_database-commentset) — 设置指定地址的注释（comment）。  （别名：commentset, cmt, cmtset）
- [dbclear](#x64dbg-user_database-dbclear) — 清空用户数据库。  （别名：dbclear, cleardb）
- [dbload](#x64dbg-user_database-dbload) — 加载用户数据库。  （别名：dbload, loaddb）
- [dbsave](#x64dbg-user_database-dbsave) — 保存用户数据库（注释/标签/书签等）。  （别名：dbsave, savedb）
- [functionadd](#x64dbg-user_database-functionadd) — 在分析数据库中添加函数范围。  （别名：functionadd, func）
- [functionclear](#x64dbg-user_database-functionclear) — 清空函数范围记录。  （别名：functionclear）
- [functiondel](#x64dbg-user_database-functiondel) — 删除函数范围记录。  （别名：functiondel, funcc）
- [functionlist](#x64dbg-user_database-functionlist) — 列出所有函数范围。  （别名：functionlist）
- [labelclear](#x64dbg-user_database-labelclear) — 清空所有标签。  （别名：labelclear）
- [labeldel](#x64dbg-user_database-labeldel) — 删除指定地址的标签。  （别名：labeldel, lblc, lbldel）
- [labellist](#x64dbg-user_database-labellist) — 列出所有标签。  （别名：labellist）
- [labelset](#x64dbg-user_database-labelset) — 设置指定地址的标签（label）。  （别名：labelset, lbl, lblset）

### argumentadd  <a id="x64dbg-user_database-argumentadd"></a>

- **Skill ID**：`x64dbg.user_database.argumentadd`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`argumentadd`
- **一句话用途**：添加函数参数信息（argument）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/argumentadd.html`

#### 什么时候用（Use cases）
- 为函数参数写入名称/类型/注释（便于后续阅读）。
- 在脚本里根据调用约定推断参数并写回 DB。
- 导出参数列表做文档化或审计。

#### 语法（Syntax）

`argumentadd <addr>, <name>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `argumentadd 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### argumentclear  <a id="x64dbg-user_database-argumentclear"></a>

- **Skill ID**：`x64dbg.user_database.argumentclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`argumentclear`
- **一句话用途**：清空函数参数信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/argumentclear.html`

#### 什么时候用（Use cases）
- 为函数参数写入名称/类型/注释（便于后续阅读）。
- 在脚本里根据调用约定推断参数并写回 DB。
- 导出参数列表做文档化或审计。

#### 语法（Syntax）

`argumentclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `argumentclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### argumentdel  <a id="x64dbg-user_database-argumentdel"></a>

- **Skill ID**：`x64dbg.user_database.argumentdel`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`argumentdel`
- **一句话用途**：删除函数参数信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/argumentdel.html`

#### 什么时候用（Use cases）
- 为函数参数写入名称/类型/注释（便于后续阅读）。
- 在脚本里根据调用约定推断参数并写回 DB。
- 导出参数列表做文档化或审计。

#### 语法（Syntax）

`argumentdel <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `argumentdel 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### argumentlist  <a id="x64dbg-user_database-argumentlist"></a>

- **Skill ID**：`x64dbg.user_database.argumentlist`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`argumentlist`
- **一句话用途**：列出所有函数参数信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/argumentlist.html`

#### 什么时候用（Use cases）
- 为函数参数写入名称/类型/注释（便于后续阅读）。
- 在脚本里根据调用约定推断参数并写回 DB。
- 导出参数列表做文档化或审计。

#### 语法（Syntax）

`argumentlist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `argumentlist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bookmarkclear  <a id="x64dbg-user_database-bookmarkclear"></a>

- **Skill ID**：`x64dbg.user_database.bookmarkclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`bookmarkclear`
- **一句话用途**：清空所有书签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/bookmarkclear.html`

#### 什么时候用（Use cases）
- 在关键地址做书签，便于快速导航。
- 脚本在发现关键点时自动 bookmark。
- 把分析流程里每一步的落点记录下来。

#### 语法（Syntax）

`bookmarkclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `bookmarkclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bookmarkdel  <a id="x64dbg-user_database-bookmarkdel"></a>

- **Skill ID**：`x64dbg.user_database.bookmarkdel`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`bookmarkdel`（别名：bookmarkc）
- **一句话用途**：删除书签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/bookmarkdel.html`

#### 什么时候用（Use cases）
- 在关键地址做书签，便于快速导航。
- 脚本在发现关键点时自动 bookmark。
- 把分析流程里每一步的落点记录下来。

#### 语法（Syntax）

`bookmarkdel <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `bookmarkdel 401000`
- `bookmarkc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bookmarklist  <a id="x64dbg-user_database-bookmarklist"></a>

- **Skill ID**：`x64dbg.user_database.bookmarklist`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`bookmarklist`
- **一句话用途**：列出所有书签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/bookmarklist.html`

#### 什么时候用（Use cases）
- 在关键地址做书签，便于快速导航。
- 脚本在发现关键点时自动 bookmark。
- 把分析流程里每一步的落点记录下来。

#### 语法（Syntax）

`bookmarklist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `bookmarklist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### bookmarkset  <a id="x64dbg-user_database-bookmarkset"></a>

- **Skill ID**：`x64dbg.user_database.bookmarkset`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`bookmarkset`（别名：bookmark）
- **一句话用途**：设置书签（bookmark）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/bookmarkset.html`

#### 什么时候用（Use cases）
- 在关键地址做书签，便于快速导航。
- 脚本在发现关键点时自动 bookmark。
- 把分析流程里每一步的落点记录下来。

#### 语法（Syntax）

`bookmarkset <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `bookmarkset 401000`
- `bookmark 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### commentclear  <a id="x64dbg-user_database-commentclear"></a>

- **Skill ID**：`x64dbg.user_database.commentclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`commentclear`
- **一句话用途**：清空所有注释。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/commentclear.html`

#### 什么时候用（Use cases）
- 为某地址设置注释/删除注释/列出注释。
- 脚本自动识别函数后批量写入注释。
- 把运行时观测到的值记录到注释中形成笔记。

#### 语法（Syntax）

`commentclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `commentclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### commentdel  <a id="x64dbg-user_database-commentdel"></a>

- **Skill ID**：`x64dbg.user_database.commentdel`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`commentdel`（别名：cmtc, cmtdel）
- **一句话用途**：删除指定地址的注释。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/commentdel.html`

#### 什么时候用（Use cases）
- 为某地址设置注释/删除注释/列出注释。
- 脚本自动识别函数后批量写入注释。
- 把运行时观测到的值记录到注释中形成笔记。

#### 语法（Syntax）

`commentdel <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `commentdel 401000`
- `cmtc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### commentlist  <a id="x64dbg-user_database-commentlist"></a>

- **Skill ID**：`x64dbg.user_database.commentlist`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`commentlist`
- **一句话用途**：列出所有注释。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/commentlist.html`

#### 什么时候用（Use cases）
- 为某地址设置注释/删除注释/列出注释。
- 脚本自动识别函数后批量写入注释。
- 把运行时观测到的值记录到注释中形成笔记。

#### 语法（Syntax）

`commentlist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `commentlist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### commentset  <a id="x64dbg-user_database-commentset"></a>

- **Skill ID**：`x64dbg.user_database.commentset`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`commentset`（别名：cmt, cmtset）
- **一句话用途**：设置指定地址的注释（comment）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/commentset.html`

#### 什么时候用（Use cases）
- 为某地址设置注释/删除注释/列出注释。
- 脚本自动识别函数后批量写入注释。
- 把运行时观测到的值记录到注释中形成笔记。

#### 语法（Syntax）

`commentset <addr>, <text>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`text`**（必需）：
  通用文本字符串。建议用引号包裹。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `commentset 401000, "text"`
- `cmt 401000, "text"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### dbclear  <a id="x64dbg-user_database-dbclear"></a>

- **Skill ID**：`x64dbg.user_database.dbclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`dbclear`（别名：cleardb）
- **一句话用途**：清空用户数据库。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/dbclear.html`

#### 什么时候用（Use cases）
- 保存/加载/清空用户数据库，持久化分析结果。
- 在批量分析前备份 DB，出错可恢复。
- 把分析状态分享给他人或在不同机器间迁移。

#### 语法（Syntax）

`dbclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 清空操作不可逆（除非你事先备份 DB/类型/配置）。

#### 示例（Examples）
- `dbclear`
- `cleardb`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### dbload  <a id="x64dbg-user_database-dbload"></a>

- **Skill ID**：`x64dbg.user_database.dbload`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`dbload`（别名：loaddb）
- **一句话用途**：加载用户数据库。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/dbload.html`

#### 什么时候用（Use cases）
- 保存/加载/清空用户数据库，持久化分析结果。
- 在批量分析前备份 DB，出错可恢复。
- 把分析状态分享给他人或在不同机器间迁移。

#### 语法（Syntax）

`dbload <file_path>`

#### 参数详解（Arguments）
- **`file_path`**（必需）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `dbload "C:\\temp\\out.bin"`
- `loaddb "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### dbsave  <a id="x64dbg-user_database-dbsave"></a>

- **Skill ID**：`x64dbg.user_database.dbsave`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`dbsave`（别名：savedb）
- **一句话用途**：保存用户数据库（注释/标签/书签等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/dbsave.html`

#### 什么时候用（Use cases）
- 保存/加载/清空用户数据库，持久化分析结果。
- 在批量分析前备份 DB，出错可恢复。
- 把分析状态分享给他人或在不同机器间迁移。

#### 语法（Syntax）

`dbsave [<file_path>]`

#### 参数详解（Arguments）
- **`file_path`**（可选）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `dbsave "C:\\temp\\out.bin"`
- `savedb "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### functionadd  <a id="x64dbg-user_database-functionadd"></a>

- **Skill ID**：`x64dbg.user_database.functionadd`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`functionadd`（别名：func）
- **一句话用途**：在分析数据库中添加函数范围。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/functionadd.html`

#### 什么时候用（Use cases）
- 创建/删除/列出函数边界（函数识别结果写入 DB）。
- 修正自动分析误判的函数范围。
- 为后续参数/类型分析提供基础。

#### 语法（Syntax）

`functionadd <start>, <end>`

#### 参数详解（Arguments）
- **`start`**（必需）：
  起始地址或起始索引（上下文相关）。
- **`end`**（必需）：
  结束地址或结束索引（上下文相关）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `functionadd 401000, 402000`
- `func 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### functionclear  <a id="x64dbg-user_database-functionclear"></a>

- **Skill ID**：`x64dbg.user_database.functionclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`functionclear`
- **一句话用途**：清空函数范围记录。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/functionclear.html`

#### 什么时候用（Use cases）
- 创建/删除/列出函数边界（函数识别结果写入 DB）。
- 修正自动分析误判的函数范围。
- 为后续参数/类型分析提供基础。

#### 语法（Syntax）

`functionclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `functionclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### functiondel  <a id="x64dbg-user_database-functiondel"></a>

- **Skill ID**：`x64dbg.user_database.functiondel`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`functiondel`（别名：funcc）
- **一句话用途**：删除函数范围记录。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/functiondel.html`

#### 什么时候用（Use cases）
- 创建/删除/列出函数边界（函数识别结果写入 DB）。
- 修正自动分析误判的函数范围。
- 为后续参数/类型分析提供基础。

#### 语法（Syntax）

`functiondel <start>`

#### 参数详解（Arguments）
- **`start`**（必需）：
  起始地址或起始索引（上下文相关）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `functiondel 401000`
- `funcc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### functionlist  <a id="x64dbg-user_database-functionlist"></a>

- **Skill ID**：`x64dbg.user_database.functionlist`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`functionlist`
- **一句话用途**：列出所有函数范围。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/functionlist.html`

#### 什么时候用（Use cases）
- 创建/删除/列出函数边界（函数识别结果写入 DB）。
- 修正自动分析误判的函数范围。
- 为后续参数/类型分析提供基础。

#### 语法（Syntax）

`functionlist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `functionlist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### labelclear  <a id="x64dbg-user_database-labelclear"></a>

- **Skill ID**：`x64dbg.user_database.labelclear`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`labelclear`
- **一句话用途**：清空所有标签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/labelclear.html`

#### 什么时候用（Use cases）
- 为某地址设置标签（符号名），提高可读性。
- 把自动识别到的函数/变量命名并写入 DB。
- 为跳转目标、重要常量地址打标。

#### 语法（Syntax）

`labelclear`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `labelclear`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### labeldel  <a id="x64dbg-user_database-labeldel"></a>

- **Skill ID**：`x64dbg.user_database.labeldel`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`labeldel`（别名：lblc, lbldel）
- **一句话用途**：删除指定地址的标签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/labeldel.html`

#### 什么时候用（Use cases）
- 为某地址设置标签（符号名），提高可读性。
- 把自动识别到的函数/变量命名并写入 DB。
- 为跳转目标、重要常量地址打标。

#### 语法（Syntax）

`labeldel <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `labeldel 401000`
- `lblc 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### labellist  <a id="x64dbg-user_database-labellist"></a>

- **Skill ID**：`x64dbg.user_database.labellist`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令**：`labellist`
- **一句话用途**：列出所有标签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/labellist.html`

#### 什么时候用（Use cases）
- 为某地址设置标签（符号名），提高可读性。
- 把自动识别到的函数/变量命名并写入 DB。
- 为跳转目标、重要常量地址打标。

#### 语法（Syntax）

`labellist`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `labellist`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### labelset  <a id="x64dbg-user_database-labelset"></a>

- **Skill ID**：`x64dbg.user_database.labelset`
- **分类**：用户数据库/标注持久化 (User Database)
- **命令/别名**：`labelset`（别名：lbl, lblset）
- **一句话用途**：设置指定地址的标签（label）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/user-database/labelset.html`

#### 什么时候用（Use cases）
- 为某地址设置标签（符号名），提高可读性。
- 把自动识别到的函数/变量命名并写入 DB。
- 为跳转目标、重要常量地址打标。

#### 语法（Syntax）

`labelset <addr>, <name>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `labelset 401000, MyName`
- `lbl 401000, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 分析 (Analysis)

分析命令用于触发或控制 x64dbg 的各类自动分析功能：
- 线性/递归反汇编、函数识别、控制流分析等（具体因命令而异）；
- 某些命令可能依赖 PDB/符号或已有 DB 状态。

注意：分析可能耗时，并且会改变“函数/基本块/标签”等内部状态；自动化时建议在分析前后做快照或导出 DB。

### 本分类命令索引

- [analadv](#x64dbg-analysis-analadv) — 高级分析选项/高级分析入口。  （别名：analadv）
- [analrecur](#x64dbg-analysis-analrecur) — 递归分析（recur）。  （别名：analrecur, analr）
- [analxrefs](#x64dbg-analysis-analxrefs) — 分析/生成交叉引用（xrefs）。  （别名：analxrefs, analx）
- [analyse](#x64dbg-analysis-analyse) — 对当前模块/范围执行分析（识别代码流、函数等）。  （别名：analyse, analyze, anal）
- [analyse_nukem](#x64dbg-analysis-analyse_nukem) — 清理/重置分析结果（nukem）。  （别名：analyse_nukem, analyze_nukem, anal_nukem）
- [cfanalyze](#x64dbg-analysis-cfanalyze) — 执行控制流分析（Control Flow Analysis）。  （别名：cfanalyze, cfanalyse, cfanal）
- [exanalyse](#x64dbg-analysis-exanalyse) — 执行扩展分析（更深入/更耗时）。  （别名：exanalyse, exanalyze, exanal）
- [exhandlers](#x64dbg-analysis-exhandlers) — 列出异常处理相关信息（exception handlers）。  （别名：exhandlers）
- [exinfo](#x64dbg-analysis-exinfo) — 显示异常信息（exception info）。  （别名：exinfo）
- [GetRelocSize](#x64dbg-analysis-getrelocsize) — 获取重定位表大小（reloc size）。  （别名：GetRelocSize, grs）
- [imageinfo](#x64dbg-analysis-imageinfo) — 显示 PE/模块的 image 信息。  （别名：imageinfo）
- [symdownload](#x64dbg-analysis-symdownload) — 下载符号（PDB 等）。  （别名：symdownload, downloadsym）
- [symload](#x64dbg-analysis-symload) — 加载符号。  （别名：symload, loadsym）
- [symunload](#x64dbg-analysis-symunload) — 卸载符号。  （别名：symunload, unloadsym）
- [traceexecute](#x64dbg-analysis-traceexecute) — 对指令执行进行 trace（execute trace）。  （别名：traceexecute）
- [virtualmod](#x64dbg-analysis-virtualmod) — 创建/管理虚拟模块（virtual module）。  （别名：virtualmod）

### analadv  <a id="x64dbg-analysis-analadv"></a>

- **Skill ID**：`x64dbg.analysis.analadv`
- **分类**：分析 (Analysis)
- **命令**：`analadv`
- **一句话用途**：高级分析选项/高级分析入口。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/analadv.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`analadv [<options>]`

#### 参数详解（Arguments）
- **`options`**（可选）：
  可选项集合（通常为逗号分隔的标志位或 key=value 对）。具体可用项依命令实现而定。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `analadv "1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### analrecur  <a id="x64dbg-analysis-analrecur"></a>

- **Skill ID**：`x64dbg.analysis.analrecur`
- **分类**：分析 (Analysis)
- **命令/别名**：`analrecur`（别名：analr）
- **一句话用途**：递归分析（recur）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/analrecur.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`analrecur [<addr>]`

#### 参数详解（Arguments）
- **`addr`**（可选）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `analrecur 401000`
- `analr 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### analxrefs  <a id="x64dbg-analysis-analxrefs"></a>

- **Skill ID**：`x64dbg.analysis.analxrefs`
- **分类**：分析 (Analysis)
- **命令/别名**：`analxrefs`（别名：analx）
- **一句话用途**：分析/生成交叉引用（xrefs）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/analxrefs.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。
- 引用（xrefs）相关的分析/刷新。

#### 语法（Syntax）

`analxrefs [<range>]`

#### 参数详解（Arguments）
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `analxrefs 401000, 402000`
- `analx 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### analyse  <a id="x64dbg-analysis-analyse"></a>

- **Skill ID**：`x64dbg.analysis.analyse`
- **分类**：分析 (Analysis)
- **命令/别名**：`analyse`（别名：analyze, anal）
- **一句话用途**：对当前模块/范围执行分析（识别代码流、函数等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/analyse.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`analyse [<range>]`

#### 参数详解（Arguments）
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `analyse 401000, 402000`
- `anal 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### analyse_nukem  <a id="x64dbg-analysis-analyse_nukem"></a>

- **Skill ID**：`x64dbg.analysis.analyse_nukem`
- **分类**：分析 (Analysis)
- **命令/别名**：`analyse_nukem`（别名：analyze_nukem, anal_nukem）
- **一句话用途**：清理/重置分析结果（nukem）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/analyse_nukem.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`analyse_nukem [<range>]`

#### 参数详解（Arguments）
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `analyse_nukem 401000, 402000`
- `anal_nukem 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### cfanalyze  <a id="x64dbg-analysis-cfanalyze"></a>

- **Skill ID**：`x64dbg.analysis.cfanalyze`
- **分类**：分析 (Analysis)
- **命令/别名**：`cfanalyze`（别名：cfanalyse, cfanal）
- **一句话用途**：执行控制流分析（Control Flow Analysis）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/cfanalyze.html`

#### 什么时候用（Use cases）
- 对当前模块或指定范围执行自动分析，以生成函数/基本块/引用等元数据。
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。
- 控制流（Control Flow）相关分析或可视化准备。

#### 语法（Syntax）

`cfanalyze [<range>]`

#### 参数详解（Arguments）
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `cfanalyze 401000, 402000`
- `cfanal 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### exanalyse  <a id="x64dbg-analysis-exanalyse"></a>

- **Skill ID**：`x64dbg.analysis.exanalyse`
- **分类**：分析 (Analysis)
- **命令/别名**：`exanalyse`（别名：exanalyze, exanal）
- **一句话用途**：执行扩展分析（更深入/更耗时）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/exanalyse.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`exanalyse [<range>]`

#### 参数详解（Arguments）
- **`range`**（可选）：
  范围参数（起始+结束 或 起始+长度）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `exanalyse 401000, 402000`
- `exanal 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### exhandlers  <a id="x64dbg-analysis-exhandlers"></a>

- **Skill ID**：`x64dbg.analysis.exhandlers`
- **分类**：分析 (Analysis)
- **命令**：`exhandlers`
- **一句话用途**：列出异常处理相关信息（exception handlers）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/exhandlers.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`exhandlers`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `exhandlers`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### exinfo  <a id="x64dbg-analysis-exinfo"></a>

- **Skill ID**：`x64dbg.analysis.exinfo`
- **分类**：分析 (Analysis)
- **命令**：`exinfo`
- **一句话用途**：显示异常信息（exception info）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/exinfo.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`exinfo`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `exinfo`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### GetRelocSize  <a id="x64dbg-analysis-getrelocsize"></a>

- **Skill ID**：`x64dbg.analysis.getrelocsize`
- **分类**：分析 (Analysis)
- **命令/别名**：`GetRelocSize`（别名：grs）
- **一句话用途**：获取重定位表大小（reloc size）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/getrelocsize.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`GetRelocSize <module>`

#### 参数详解（Arguments）
- **`module`**（必需）：
  模块名（如 user32.dll）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `GetRelocSize kernel32.dll`
- `grs kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### imageinfo  <a id="x64dbg-analysis-imageinfo"></a>

- **Skill ID**：`x64dbg.analysis.imageinfo`
- **分类**：分析 (Analysis)
- **命令**：`imageinfo`
- **一句话用途**：显示 PE/模块的 image 信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/imageinfo.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`imageinfo <module_or_addr>`

#### 参数详解（Arguments）
- **`module_or_addr`**（必需）：
  模块名或地址。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `imageinfo kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### symdownload  <a id="x64dbg-analysis-symdownload"></a>

- **Skill ID**：`x64dbg.analysis.symdownload`
- **分类**：分析 (Analysis)
- **命令/别名**：`symdownload`（别名：downloadsym）
- **一句话用途**：下载符号（PDB 等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/symdownload.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`symdownload <module>`

#### 参数详解（Arguments）
- **`module`**（必需）：
  模块名（如 user32.dll）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `symdownload kernel32.dll`
- `downloadsym kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### symload  <a id="x64dbg-analysis-symload"></a>

- **Skill ID**：`x64dbg.analysis.symload`
- **分类**：分析 (Analysis)
- **命令/别名**：`symload`（别名：loadsym）
- **一句话用途**：加载符号。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/symload.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`symload <module_or_pdb>`

#### 参数详解（Arguments）
- **`module_or_pdb`**（必需）：
  模块名或 PDB 路径/标识。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `symload "C:\\symbols\\my.pdb"`
- `loadsym "C:\\symbols\\my.pdb"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### symunload  <a id="x64dbg-analysis-symunload"></a>

- **Skill ID**：`x64dbg.analysis.symunload`
- **分类**：分析 (Analysis)
- **命令/别名**：`symunload`（别名：unloadsym）
- **一句话用途**：卸载符号。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/symunload.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`symunload <module>`

#### 参数详解（Arguments）
- **`module`**（必需）：
  模块名（如 user32.dll）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `symunload kernel32.dll`
- `unloadsym kernel32.dll`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### traceexecute  <a id="x64dbg-analysis-traceexecute"></a>

- **Skill ID**：`x64dbg.analysis.traceexecute`
- **分类**：分析 (Analysis)
- **命令**：`traceexecute`
- **一句话用途**：对指令执行进行 trace（execute trace）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/traceexecute.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`traceexecute <addr_or_range>[, <count>]`

#### 参数详解（Arguments）
- **`addr_or_range`**（必需）：
  地址或范围。
- **`count`**（可选）：
  移位/旋转次数。通常取低 5 位(32-bit)或低 6 位(64-bit)；建议显式限制在 0~63 以内以避免歧义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `traceexecute 401000, 402000`
- `traceexecute 401000, 402000, 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### virtualmod  <a id="x64dbg-analysis-virtualmod"></a>

- **Skill ID**：`x64dbg.analysis.virtualmod`
- **分类**：分析 (Analysis)
- **命令**：`virtualmod`
- **一句话用途**：创建/管理虚拟模块（virtual module）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/analysis/virtualmod.html`

#### 什么时候用（Use cases）
- 触发/控制自动分析流程（函数识别、控制流分析等）。
- 在脚本中对某个区域/模块执行分析后再做标注或搜索。
- 对分析结果进行刷新/清理/重建（具体依命令实现）。

#### 语法（Syntax）

`virtualmod <base>, <size>[, <name>]`

#### 参数详解（Arguments）
- **`base`**（必需）：
  基地址（VA）或模块基址表达式。
- **`size`**（必需）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。
- **`name`**（可选）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `virtualmod 401000, 10`
- `virtualmod 401000, 10, MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 类型系统 (Types)

类型系统命令用于：
- 把某段内存解释为特定数据类型（DataByte/DataWord/.../DataUnicode 等）；
- 创建/维护自定义类型数据库：AddType/AddStruct/AddUnion/AddMember/AddFunction/AddArg…；
- dt/VisitType 展示类型；
- LoadTypes/ParseTypes 从文本或文件导入类型。

这对于结构体逆向、解析复杂数据结构非常关键。

### 本分类命令索引

- [AddArg](#x64dbg-types-addarg) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddArg）
- [AddFunction](#x64dbg-types-addfunction) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddFunction）
- [AddMember](#x64dbg-types-addmember) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddMember）
- [AddStruct](#x64dbg-types-addstruct) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddStruct）
- [AddType](#x64dbg-types-addtype) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddType）
- [AddUnion](#x64dbg-types-addunion) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AddUnion）
- [AppendArg](#x64dbg-types-appendarg) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AppendArg）
- [AppendMember](#x64dbg-types-appendmember) — 添加/扩展自定义类型定义（结构体/联合体/函数签名等）。  （别名：AppendMember）
- [ClearTypes](#x64dbg-types-cleartypes) — 清空所有自定义类型定义。  （别名：ClearTypes）
- [DataAscii](#x64dbg-types-dataascii) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataAscii, da）
- [DataByte](#x64dbg-types-databyte) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataByte, db）
- [DataCode](#x64dbg-types-datacode) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataCode, dc）
- [DataDouble](#x64dbg-types-datadouble) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataDouble, DataReal8）
- [DataDword](#x64dbg-types-datadword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataDword, dd）
- [DataFloat](#x64dbg-types-datafloat) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataFloat, DataReal4, df）
- [DataFword](#x64dbg-types-datafword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataFword）
- [DataJunk](#x64dbg-types-datajunk) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataJunk）
- [DataLongdouble](#x64dbg-types-datalongdouble) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataLongdouble, DataReal10）
- [DataMiddle](#x64dbg-types-datamiddle) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataMiddle）
- [DataMmword](#x64dbg-types-datammword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataMmword）
- [DataOword](#x64dbg-types-dataoword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataOword）
- [DataQword](#x64dbg-types-dataqword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataQword, dq）
- [DataTbyte](#x64dbg-types-datatbyte) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataTbyte）
- [DataUnicode](#x64dbg-types-dataunicode) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataUnicode, du）
- [DataUnknown](#x64dbg-types-dataunknown) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataUnknown）
- [DataWord](#x64dbg-types-dataword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataWord, dw）
- [DataXmmword](#x64dbg-types-dataxmmword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataXmmword）
- [DataYmmword](#x64dbg-types-dataymmword) — 将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。  （别名：DataYmmword）
- [EnumTypes](#x64dbg-types-enumtypes) — 枚举当前已加载的类型。  （别名：EnumTypes）
- [LoadTypes](#x64dbg-types-loadtypes) — 从文件加载类型定义。  （别名：LoadTypes）
- [ParseTypes](#x64dbg-types-parsetypes) — 解析/导入类型定义文本。  （别名：ParseTypes）
- [RemoveType](#x64dbg-types-removetype) — 移除某个类型定义。  （别名：RemoveType）
- [SizeofType](#x64dbg-types-sizeoftype) — 计算某类型的大小（sizeof）。  （别名：SizeofType）
- [VisitType](#x64dbg-types-visittype) — 显示/展开某类型（类似 dt 命令）。  （别名：VisitType, DisplayType, dt）

### AddArg  <a id="x64dbg-types-addarg"></a>

- **Skill ID**：`x64dbg.types.addarg`
- **分类**：类型系统 (Types)
- **命令**：`AddArg`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addarg.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddArg <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddArg "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddFunction  <a id="x64dbg-types-addfunction"></a>

- **Skill ID**：`x64dbg.types.addfunction`
- **分类**：类型系统 (Types)
- **命令**：`AddFunction`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addfunction.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddFunction <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddFunction "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddMember  <a id="x64dbg-types-addmember"></a>

- **Skill ID**：`x64dbg.types.addmember`
- **分类**：类型系统 (Types)
- **命令**：`AddMember`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addmember.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddMember <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddMember "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddStruct  <a id="x64dbg-types-addstruct"></a>

- **Skill ID**：`x64dbg.types.addstruct`
- **分类**：类型系统 (Types)
- **命令**：`AddStruct`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addstruct.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddStruct <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddStruct "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddType  <a id="x64dbg-types-addtype"></a>

- **Skill ID**：`x64dbg.types.addtype`
- **分类**：类型系统 (Types)
- **命令**：`AddType`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addtype.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddType <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddType "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddUnion  <a id="x64dbg-types-addunion"></a>

- **Skill ID**：`x64dbg.types.addunion`
- **分类**：类型系统 (Types)
- **命令**：`AddUnion`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/addunion.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AddUnion <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddUnion "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AppendArg  <a id="x64dbg-types-appendarg"></a>

- **Skill ID**：`x64dbg.types.appendarg`
- **分类**：类型系统 (Types)
- **命令**：`AppendArg`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/appendarg.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AppendArg <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AppendArg "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AppendMember  <a id="x64dbg-types-appendmember"></a>

- **Skill ID**：`x64dbg.types.appendmember`
- **分类**：类型系统 (Types)
- **命令**：`AppendMember`
- **一句话用途**：添加/扩展自定义类型定义（结构体/联合体/函数签名等）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/appendmember.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`AppendMember <type_definition_or_parts>`

#### 参数详解（Arguments）
- **`type_definition_or_parts`**（必需）：
  类型定义文本或分段参数（取决于实现）。常用于 AddType/ParseTypes 这类把文本解析成类型数据库的命令。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AppendMember "typedef struct { int a; } MY_STRUCT;"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ClearTypes  <a id="x64dbg-types-cleartypes"></a>

- **Skill ID**：`x64dbg.types.cleartypes`
- **分类**：类型系统 (Types)
- **命令**：`ClearTypes`
- **一句话用途**：清空所有自定义类型定义。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/cleartypes.html`

#### 什么时候用（Use cases）
- 查看/枚举/删除/清空类型数据库。
- 在脚本里做“类型存在性检查”，避免重复添加。
- 把类型系统作为自动化分析的输入与输出接口。

#### 语法（Syntax）

`ClearTypes`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 清空操作不可逆（除非你事先备份 DB/类型/配置）。

#### 示例（Examples）
- `ClearTypes`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataAscii  <a id="x64dbg-types-dataascii"></a>

- **Skill ID**：`x64dbg.types.dataascii`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataAscii`（别名：da）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataascii.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataAscii <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataAscii 401000`
- `da 401000`
- `DataAscii 401000, 10`
- `da 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataByte  <a id="x64dbg-types-databyte"></a>

- **Skill ID**：`x64dbg.types.databyte`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataByte`（别名：db）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/databyte.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataByte <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataByte 401000`
- `db 401000`
- `DataByte 401000, 10`
- `db 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataCode  <a id="x64dbg-types-datacode"></a>

- **Skill ID**：`x64dbg.types.datacode`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataCode`（别名：dc）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datacode.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataCode <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataCode 401000`
- `dc 401000`
- `DataCode 401000, 10`
- `dc 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataDouble  <a id="x64dbg-types-datadouble"></a>

- **Skill ID**：`x64dbg.types.datadouble`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataDouble`（别名：DataReal8）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datadouble.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataDouble <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataDouble 401000`
- `DataReal8 401000`
- `DataDouble 401000, 10`
- `DataReal8 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataDword  <a id="x64dbg-types-datadword"></a>

- **Skill ID**：`x64dbg.types.datadword`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataDword`（别名：dd）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datadword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataDword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataDword 401000`
- `dd 401000`
- `DataDword 401000, 10`
- `dd 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataFloat  <a id="x64dbg-types-datafloat"></a>

- **Skill ID**：`x64dbg.types.datafloat`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataFloat`（别名：DataReal4, df）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datafloat.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataFloat <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataFloat 401000`
- `df 401000`
- `DataFloat 401000, 10`
- `df 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataFword  <a id="x64dbg-types-datafword"></a>

- **Skill ID**：`x64dbg.types.datafword`
- **分类**：类型系统 (Types)
- **命令**：`DataFword`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datafword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataFword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataFword 401000`
- `DataFword 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataJunk  <a id="x64dbg-types-datajunk"></a>

- **Skill ID**：`x64dbg.types.datajunk`
- **分类**：类型系统 (Types)
- **命令**：`DataJunk`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datajunk.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataJunk <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataJunk 401000`
- `DataJunk 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataLongdouble  <a id="x64dbg-types-datalongdouble"></a>

- **Skill ID**：`x64dbg.types.datalongdouble`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataLongdouble`（别名：DataReal10）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datalongdouble.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataLongdouble <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataLongdouble 401000`
- `DataReal10 401000`
- `DataLongdouble 401000, 10`
- `DataReal10 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataMiddle  <a id="x64dbg-types-datamiddle"></a>

- **Skill ID**：`x64dbg.types.datamiddle`
- **分类**：类型系统 (Types)
- **命令**：`DataMiddle`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datamiddle.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataMiddle <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataMiddle 401000`
- `DataMiddle 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataMmword  <a id="x64dbg-types-datammword"></a>

- **Skill ID**：`x64dbg.types.datammword`
- **分类**：类型系统 (Types)
- **命令**：`DataMmword`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datammword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataMmword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataMmword 401000`
- `DataMmword 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataOword  <a id="x64dbg-types-dataoword"></a>

- **Skill ID**：`x64dbg.types.dataoword`
- **分类**：类型系统 (Types)
- **命令**：`DataOword`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataoword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataOword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataOword 401000`
- `DataOword 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataQword  <a id="x64dbg-types-dataqword"></a>

- **Skill ID**：`x64dbg.types.dataqword`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataQword`（别名：dq）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataqword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataQword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataQword 401000`
- `dq 401000`
- `DataQword 401000, 10`
- `dq 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataTbyte  <a id="x64dbg-types-datatbyte"></a>

- **Skill ID**：`x64dbg.types.datatbyte`
- **分类**：类型系统 (Types)
- **命令**：`DataTbyte`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/datatbyte.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataTbyte <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataTbyte 401000`
- `DataTbyte 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataUnicode  <a id="x64dbg-types-dataunicode"></a>

- **Skill ID**：`x64dbg.types.dataunicode`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataUnicode`（别名：du）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataunicode.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataUnicode <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataUnicode 401000`
- `du 401000`
- `DataUnicode 401000, 10`
- `du 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataUnknown  <a id="x64dbg-types-dataunknown"></a>

- **Skill ID**：`x64dbg.types.dataunknown`
- **分类**：类型系统 (Types)
- **命令**：`DataUnknown`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataunknown.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataUnknown <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataUnknown 401000`
- `DataUnknown 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataWord  <a id="x64dbg-types-dataword"></a>

- **Skill ID**：`x64dbg.types.dataword`
- **分类**：类型系统 (Types)
- **命令/别名**：`DataWord`（别名：dw）
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataWord <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataWord 401000`
- `dw 401000`
- `DataWord 401000, 10`
- `dw 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataXmmword  <a id="x64dbg-types-dataxmmword"></a>

- **Skill ID**：`x64dbg.types.dataxmmword`
- **分类**：类型系统 (Types)
- **命令**：`DataXmmword`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataxmmword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataXmmword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataXmmword 401000`
- `DataXmmword 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DataYmmword  <a id="x64dbg-types-dataymmword"></a>

- **Skill ID**：`x64dbg.types.dataymmword`
- **分类**：类型系统 (Types)
- **命令**：`DataYmmword`
- **一句话用途**：将当前地址处的数据解释/标注为指定数据类型（影响反汇编/数据显示）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/dataymmword.html`

#### 什么时候用（Use cases）
- 把某地址处的数据解释成指定类型（byte/word/dword/qword/float/ascii/unicode…）。
- 在 dump/反汇编中更直观地查看数据结构。
- 修正自动识别的数据类型（例如把 code 误识别成 junk）。

#### 语法（Syntax）

`DataYmmword <addr>[, <size>]`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`size`**（可选）：
  大小/长度（字节数）。用于内存拷贝/填充/断点范围等。建议明确写十进制或十六进制并在注释中标明单位。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 可能写寄存器/内存/数据库；若参数错误可能破坏现场或导致崩溃。

#### 示例（Examples）
- `DataYmmword 401000`
- `DataYmmword 401000, 10`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnumTypes  <a id="x64dbg-types-enumtypes"></a>

- **Skill ID**：`x64dbg.types.enumtypes`
- **分类**：类型系统 (Types)
- **命令**：`EnumTypes`
- **一句话用途**：枚举当前已加载的类型。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/enumtypes.html`

#### 什么时候用（Use cases）
- 查看/枚举/删除/清空类型数据库。
- 在脚本里做“类型存在性检查”，避免重复添加。
- 把类型系统作为自动化分析的输入与输出接口。

#### 语法（Syntax）

`EnumTypes`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnumTypes`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### LoadTypes  <a id="x64dbg-types-loadtypes"></a>

- **Skill ID**：`x64dbg.types.loadtypes`
- **分类**：类型系统 (Types)
- **命令**：`LoadTypes`
- **一句话用途**：从文件加载类型定义。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/loadtypes.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`LoadTypes <file_path>`

#### 参数详解（Arguments）
- **`file_path`**（必需）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `LoadTypes "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ParseTypes  <a id="x64dbg-types-parsetypes"></a>

- **Skill ID**：`x64dbg.types.parsetypes`
- **分类**：类型系统 (Types)
- **命令**：`ParseTypes`
- **一句话用途**：解析/导入类型定义文本。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/parsetypes.html`

#### 什么时候用（Use cases）
- 建立/扩展自定义类型数据库（struct/union/function/prototype）。
- 把外部头文件/类型定义导入到 x64dbg，提升数据结构可读性。
- 为后续 dt/VisitType 展示与内存解释提供基础。

#### 语法（Syntax）

`ParseTypes <text_or_file>`

#### 参数详解（Arguments）
- **`text_or_file`**（必需）：
  文本或文件（取决于命令支持：直接传文本，或传文件路径让命令读取）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ParseTypes "C:\\temp\\types.txt"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### RemoveType  <a id="x64dbg-types-removetype"></a>

- **Skill ID**：`x64dbg.types.removetype`
- **分类**：类型系统 (Types)
- **命令**：`RemoveType`
- **一句话用途**：移除某个类型定义。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/removetype.html`

#### 什么时候用（Use cases）
- 查看/枚举/删除/清空类型数据库。
- 在脚本里做“类型存在性检查”，避免重复添加。
- 把类型系统作为自动化分析的输入与输出接口。

#### 语法（Syntax）

`RemoveType <type_name>`

#### 参数详解（Arguments）
- **`type_name`**（必需）：
  类型名（Type）。可以是内置基础类型或你通过 AddType/AddStruct 等注册的自定义类型。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `RemoveType MY_STRUCT`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### SizeofType  <a id="x64dbg-types-sizeoftype"></a>

- **Skill ID**：`x64dbg.types.sizeoftype`
- **分类**：类型系统 (Types)
- **命令**：`SizeofType`
- **一句话用途**：计算某类型的大小（sizeof）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/sizeoftype.html`

#### 什么时候用（Use cases）
- 查看/枚举/删除/清空类型数据库。
- 在脚本里做“类型存在性检查”，避免重复添加。
- 把类型系统作为自动化分析的输入与输出接口。

#### 语法（Syntax）

`SizeofType <type_name>`

#### 参数详解（Arguments）
- **`type_name`**（必需）：
  类型名（Type）。可以是内置基础类型或你通过 AddType/AddStruct 等注册的自定义类型。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `SizeofType MY_STRUCT`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### VisitType  <a id="x64dbg-types-visittype"></a>

- **Skill ID**：`x64dbg.types.visittype`
- **分类**：类型系统 (Types)
- **命令/别名**：`VisitType`（别名：DisplayType, dt）
- **一句话用途**：显示/展开某类型（类似 dt 命令）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/types/visittype.html`

#### 什么时候用（Use cases）
- 查看/枚举/删除/清空类型数据库。
- 在脚本里做“类型存在性检查”，避免重复添加。
- 把类型系统作为自动化分析的输入与输出接口。

#### 语法（Syntax）

`VisitType <type_name>`

#### 参数详解（Arguments）
- **`type_name`**（必需）：
  类型名（Type）。可以是内置基础类型或你通过 AddType/AddStruct 等注册的自定义类型。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。
- 推荐在 **暂停态** 执行（pause 或断点命中后），以避免运行态改内存/线程导致不一致。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `VisitType MY_STRUCT`
- `dt MY_STRUCT`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 插件 (Plugins)

插件命令用于加载/卸载插件，以及启动一些集成插件（例如 Scylla/Import Reconstructor）。
自动化时注意插件名/路径、以及插件命令是否要求 GUI 线程/暂停态。

### 本分类命令索引

- [plugload](#x64dbg-plugins-plugload) — 加载插件（plugin）。  （别名：plugload, pluginload, loadplugin）
- [plugunload](#x64dbg-plugins-plugunload) — 卸载插件。  （别名：plugunload, pluginunload, unloadplugin）
- [StartScylla](#x64dbg-plugins-startscylla) — 启动/调用内置 Scylla（常用于 IAT 修复/导入重建）。  （别名：StartScylla, scylla, imprec）

### plugload  <a id="x64dbg-plugins-plugload"></a>

- **Skill ID**：`x64dbg.plugins.plugload`
- **分类**：插件 (Plugins)
- **命令/别名**：`plugload`（别名：pluginload, loadplugin）
- **一句话用途**：加载插件（plugin）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/plugins/plugload.html`

#### 什么时候用（Use cases）
- 在运行时加载/卸载插件，扩展 x64dbg 功能。
- 自动化环境初始化：启动时加载必要插件。
- 在测试插件兼容性时做动态启停。

#### 语法（Syntax）

`plugload <path_or_name>`

#### 参数详解（Arguments）
- **`path_or_name`**（必需）：
  路径或名称。用于加载插件/脚本等。

#### 执行上下文 / 前置条件（Preconditions）
- 插件文件/名称应可解析；若是路径含空格请加引号。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `plugload "C:\\temp\\a.bin"`
- `loadplugin "C:\\temp\\a.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### plugunload  <a id="x64dbg-plugins-plugunload"></a>

- **Skill ID**：`x64dbg.plugins.plugunload`
- **分类**：插件 (Plugins)
- **命令/别名**：`plugunload`（别名：pluginunload, unloadplugin）
- **一句话用途**：卸载插件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/plugins/plugunload.html`

#### 什么时候用（Use cases）
- 在运行时加载/卸载插件，扩展 x64dbg 功能。
- 自动化环境初始化：启动时加载必要插件。
- 在测试插件兼容性时做动态启停。

#### 语法（Syntax）

`plugunload <name>`

#### 参数详解（Arguments）
- **`name`**（必需）：
  名称字符串。用于给断点/类型/结构体/线程等命名。建议：
  - 简短；
  - 避免空格或用引号；
  - 保持可复用（便于脚本引用）。

#### 执行上下文 / 前置条件（Preconditions）
- 插件文件/名称应可解析；若是路径含空格请加引号。
- ⚠️ 这是高影响/可能破坏现场的操作；建议在自动化里提供确认开关或 dry-run。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `plugunload MyName`
- `pluginunload MyName`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### StartScylla  <a id="x64dbg-plugins-startscylla"></a>

- **Skill ID**：`x64dbg.plugins.startscylla`
- **分类**：插件 (Plugins)
- **命令/别名**：`StartScylla`（别名：scylla, imprec）
- **一句话用途**：启动/调用内置 Scylla（常用于 IAT 修复/导入重建）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/plugins/startscylla.html`

#### 什么时候用（Use cases）
- 调用集成插件（例如 Scylla）处理特定任务（IAT/导入修复等）。
- 在脱壳/导入重建流程中一键启动插件 UI/功能。
- 脚本化调用插件，减少手工操作。

#### 语法（Syntax）

`StartScylla [<options>]`

#### 参数详解（Arguments）
- **`options`**（可选）：
  可选项集合（通常为逗号分隔的标志位或 key=value 对）。具体可用项依命令实现而定。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `StartScylla "1"`
- `imprec "1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 脚本命令 (Script Commands)

脚本命令是“脚本引擎控制流”的基础：
- call/ret；
- IFxx/Jxx 条件与跳转；
- log/msg/msgyn/pause；
- 加载/执行脚本（scriptload/scriptrun/scriptexec），以及 DLLScript 等。

如果你的 Agent 需要执行多步策略，建议把高层策略编译成脚本，再用这些命令驱动执行。

### 本分类命令索引

- [call](#x64dbg-script_commands-call) — 在脚本中调用（call）某段脚本/过程/标签。  （别名：call）
- [error](#x64dbg-script_commands-error) — 打印/抛出脚本错误（error）。  （别名：error）
- [invalid](#x64dbg-script_commands-invalid) — 触发脚本错误（invalid）。  （别名：invalid）
- [Jxx](#x64dbg-script_commands-jxx) — 脚本条件跳转（Jxx/IFxx，按条件跳到标签）。  （别名：Jxx, IFxx）
- [log](#x64dbg-script_commands-log) — 在脚本中写日志。  （别名：log）
- [msg](#x64dbg-script_commands-msg) — 弹出消息框（脚本）。  （别名：msg）
- [msgyn](#x64dbg-script_commands-msgyn) — 弹出 Yes/No 消息框并获取选择。  （别名：msgyn）
- [pause](#x64dbg-script_commands-pause) — 暂停被调试进程（break/pause）。  （别名：pause）
- [printstack](#x64dbg-script_commands-printstack) — 输出栈信息（脚本）。  （别名：printstack）
- [ret](#x64dbg-script_commands-ret) — 从脚本过程返回。  （别名：ret）
- [scriptcmd](#x64dbg-script_commands-scriptcmd) — 在脚本里执行一条命令行命令。  （别名：scriptcmd）
- [scriptdll](#x64dbg-script_commands-scriptdll) — 从 DLL 加载/执行脚本接口。  （别名：scriptdll, dllscript）
- [scriptexec](#x64dbg-script_commands-scriptexec) — 直接执行脚本文本。  （别名：scriptexec）
- [scriptload](#x64dbg-script_commands-scriptload) — 加载脚本文件。  （别名：scriptload）
- [scriptrun](#x64dbg-script_commands-scriptrun) — 运行已加载脚本。  （别名：scriptrun）

### call  <a id="x64dbg-script_commands-call"></a>

- **Skill ID**：`x64dbg.script_commands.call`
- **分类**：脚本命令 (Script Commands)
- **命令**：`call`
- **一句话用途**：在脚本中调用（call）某段脚本/过程/标签。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/call.html`

#### 什么时候用（Use cases）
- 在脚本中实现子过程/函数调用（call/ret）。
- 把复杂自动化拆分成多个可复用脚本块。
- 配合变量与 IFxx/Jxx 实现结构化脚本。

#### 语法（Syntax）

`call <label>`

#### 参数详解（Arguments）
- **`label`**（必需）：
  标签名。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `call MyLabel`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### error  <a id="x64dbg-script_commands-error"></a>

- **Skill ID**：`x64dbg.script_commands.error`
- **分类**：脚本命令 (Script Commands)
- **命令**：`error`
- **一句话用途**：打印/抛出脚本错误（error）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/error.html`

#### 什么时候用（Use cases）
- 脚本引擎控制、日志与交互。
- 用于构建复杂自动化调试流程。
- 注意脚本执行时机与目标进程状态。

#### 语法（Syntax）

`error <message>`

#### 参数详解（Arguments）
- **`message`**（必需）：
  要显示给用户的消息文本（msg/msgyn 等）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `error "Hello from x64dbg"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### invalid  <a id="x64dbg-script_commands-invalid"></a>

- **Skill ID**：`x64dbg.script_commands.invalid`
- **分类**：脚本命令 (Script Commands)
- **命令**：`invalid`
- **一句话用途**：触发脚本错误（invalid）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/invalid.html`

#### 什么时候用（Use cases）
- 脚本引擎控制、日志与交互。
- 用于构建复杂自动化调试流程。
- 注意脚本执行时机与目标进程状态。

#### 语法（Syntax）

`invalid`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `invalid`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### Jxx  <a id="x64dbg-script_commands-jxx"></a>

- **Skill ID**：`x64dbg.script_commands.jxx`
- **分类**：脚本命令 (Script Commands)
- **命令/别名**：`Jxx`（别名：IFxx）
- **一句话用途**：脚本条件跳转（Jxx/IFxx，按条件跳到标签）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/jxx.html`

#### 什么时候用（Use cases）
- 条件判断与跳转：实现脚本控制流（if/else/loop）。
- 在断点命中脚本中做条件分支（例如根据寄存器值决定是否继续 run）。
- 实现有限状态机（FSM）式自动化调试策略。

#### 语法（Syntax）

`Jxx <label>  / IFxx <label>`

#### 参数详解（Arguments）
- **`label`**（必需）：
  标签名。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `Jxx MyLabel`
- `IFxx MyLabel`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### log  <a id="x64dbg-script_commands-log"></a>

- **Skill ID**：`x64dbg.script_commands.log`
- **分类**：脚本命令 (Script Commands)
- **命令**：`log`
- **一句话用途**：在脚本中写日志。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/log.html`

#### 什么时候用（Use cases）
- 输出日志/消息、与用户交互确认（msgyn）、暂停脚本。
- 在自动化过程中打印栈/记录状态，便于回溯。
- 在关键步骤插入人工检查点（pause）。

#### 语法（Syntax）

`log <text>`

#### 参数详解（Arguments）
- **`text`**（必需）：
  通用文本字符串。建议用引号包裹。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `log "text"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### msg  <a id="x64dbg-script_commands-msg"></a>

- **Skill ID**：`x64dbg.script_commands.msg`
- **分类**：脚本命令 (Script Commands)
- **命令**：`msg`
- **一句话用途**：弹出消息框（脚本）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/msg.html`

#### 什么时候用（Use cases）
- 输出日志/消息、与用户交互确认（msgyn）、暂停脚本。
- 在自动化过程中打印栈/记录状态，便于回溯。
- 在关键步骤插入人工检查点（pause）。

#### 语法（Syntax）

`msg <text>`

#### 参数详解（Arguments）
- **`text`**（必需）：
  通用文本字符串。建议用引号包裹。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `msg "text"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### msgyn  <a id="x64dbg-script_commands-msgyn"></a>

- **Skill ID**：`x64dbg.script_commands.msgyn`
- **分类**：脚本命令 (Script Commands)
- **命令**：`msgyn`
- **一句话用途**：弹出 Yes/No 消息框并获取选择。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/msgyn.html`

#### 什么时候用（Use cases）
- 输出日志/消息、与用户交互确认（msgyn）、暂停脚本。
- 在自动化过程中打印栈/记录状态，便于回溯。
- 在关键步骤插入人工检查点（pause）。

#### 语法（Syntax）

`msgyn <text>`

#### 参数详解（Arguments）
- **`text`**（必需）：
  通用文本字符串。建议用引号包裹。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `msgyn "text"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### pause  <a id="x64dbg-script_commands-pause"></a>

- **Skill ID**：`x64dbg.script_commands.pause`
- **分类**：脚本命令 (Script Commands)
- **命令**：`pause`
- **一句话用途**：暂停被调试进程（break/pause）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/pause.html`

#### 什么时候用（Use cases）
- 输出日志/消息、与用户交互确认（msgyn）、暂停脚本。
- 在自动化过程中打印栈/记录状态，便于回溯。
- 在关键步骤插入人工检查点（pause）。

#### 语法（Syntax）

`pause`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `pause`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### printstack  <a id="x64dbg-script_commands-printstack"></a>

- **Skill ID**：`x64dbg.script_commands.printstack`
- **分类**：脚本命令 (Script Commands)
- **命令**：`printstack`
- **一句话用途**：输出栈信息（脚本）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/printstack.html`

#### 什么时候用（Use cases）
- 输出日志/消息、与用户交互确认（msgyn）、暂停脚本。
- 在自动化过程中打印栈/记录状态，便于回溯。
- 在关键步骤插入人工检查点（pause）。

#### 语法（Syntax）

`printstack`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `printstack`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ret  <a id="x64dbg-script_commands-ret"></a>

- **Skill ID**：`x64dbg.script_commands.ret`
- **分类**：脚本命令 (Script Commands)
- **命令**：`ret`
- **一句话用途**：从脚本过程返回。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/ret.html`

#### 什么时候用（Use cases）
- 在脚本中实现子过程/函数调用（call/ret）。
- 把复杂自动化拆分成多个可复用脚本块。
- 配合变量与 IFxx/Jxx 实现结构化脚本。

#### 语法（Syntax）

`ret`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ret`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### scriptcmd  <a id="x64dbg-script_commands-scriptcmd"></a>

- **Skill ID**：`x64dbg.script_commands.scriptcmd`
- **分类**：脚本命令 (Script Commands)
- **命令**：`scriptcmd`
- **一句话用途**：在脚本里执行一条命令行命令。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/scriptcmd.html`

#### 什么时候用（Use cases）
- 加载/运行/执行脚本内容（文件或文本）。
- 把 Agent 的高层策略编译成脚本文本，然后一次性执行。
- 通过 scriptcmd 把命令作为脚本语句执行，便于组合。

#### 语法（Syntax）

`scriptcmd <command_line>`

#### 参数详解（Arguments）
- **`command_line`**（必需）：
  命令行文本（通常是一整段字符串）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `scriptcmd "bplist"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### scriptdll  <a id="x64dbg-script_commands-scriptdll"></a>

- **Skill ID**：`x64dbg.script_commands.scriptdll`
- **分类**：脚本命令 (Script Commands)
- **命令/别名**：`scriptdll`（别名：dllscript）
- **一句话用途**：从 DLL 加载/执行脚本接口。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/scriptdll.html`

#### 什么时候用（Use cases）
- 加载/运行/执行脚本内容（文件或文本）。
- 把 Agent 的高层策略编译成脚本文本，然后一次性执行。
- 通过 scriptcmd 把命令作为脚本语句执行，便于组合。

#### 语法（Syntax）

`scriptdll <dll_path>`

#### 参数详解（Arguments）
- **`dll_path`**（必需）：
  DLL 路径。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `scriptdll "C:\\Windows\\System32\\user32.dll"`
- `dllscript "C:\\Windows\\System32\\user32.dll"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### scriptexec  <a id="x64dbg-script_commands-scriptexec"></a>

- **Skill ID**：`x64dbg.script_commands.scriptexec`
- **分类**：脚本命令 (Script Commands)
- **命令**：`scriptexec`
- **一句话用途**：直接执行脚本文本。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/scriptexec.html`

#### 什么时候用（Use cases）
- 加载/运行/执行脚本内容（文件或文本）。
- 把 Agent 的高层策略编译成脚本文本，然后一次性执行。
- 通过 scriptcmd 把命令作为脚本语句执行，便于组合。

#### 语法（Syntax）

`scriptexec <script_text>`

#### 参数详解（Arguments）
- **`script_text`**（必需）：
  脚本文本。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `scriptexec "log \"hi\"; pause"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### scriptload  <a id="x64dbg-script_commands-scriptload"></a>

- **Skill ID**：`x64dbg.script_commands.scriptload`
- **分类**：脚本命令 (Script Commands)
- **命令**：`scriptload`
- **一句话用途**：加载脚本文件。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/scriptload.html`

#### 什么时候用（Use cases）
- 加载/运行/执行脚本内容（文件或文本）。
- 把 Agent 的高层策略编译成脚本文本，然后一次性执行。
- 通过 scriptcmd 把命令作为脚本语句执行，便于组合。

#### 语法（Syntax）

`scriptload <file_path>`

#### 参数详解（Arguments）
- **`file_path`**（必需）：
  文件路径（读/写）。建议绝对路径；含空格加引号。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `scriptload "C:\\temp\\out.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### scriptrun  <a id="x64dbg-script_commands-scriptrun"></a>

- **Skill ID**：`x64dbg.script_commands.scriptrun`
- **分类**：脚本命令 (Script Commands)
- **命令**：`scriptrun`
- **一句话用途**：运行已加载脚本。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/script/scriptrun.html`

#### 什么时候用（Use cases）
- 加载/运行/执行脚本内容（文件或文本）。
- 把 Agent 的高层策略编译成脚本文本，然后一次性执行。
- 通过 scriptcmd 把命令作为脚本语句执行，便于组合。

#### 语法（Syntax）

`scriptrun`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `scriptrun`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## GUI/界面控制 (GUI)

GUI 命令主要用于控制/更新界面与引用视图：
- 打开/切换到反汇编、dump、stack dump、memmap dump、graph；
- 控制 GUI 更新开关（避免批量操作时闪烁/耗时）；
- 引用视图 refinit/refadd/refget；
- 日志窗口 enable/disable/clear；
- 收藏工具/命令、设置快捷键；
- 折叠反汇编视图等。

### 本分类命令索引

- [AddFavouriteCommand](#x64dbg-gui-addfavouritecommand) — 将某命令添加到 Favorites。  （别名：AddFavouriteCommand）
- [AddFavouriteTool](#x64dbg-gui-addfavouritetool) — 将某工具添加到 Favorites。  （别名：AddFavouriteTool）
- [AddFavouriteToolShortcut](#x64dbg-gui-addfavouritetoolshortcut) — 为某 Favorite 工具设置快捷键。  （别名：AddFavouriteToolShortcut, SetFavouriteToolShortcut）
- [ClearLog](#x64dbg-gui-clearlog) — 清空 Log 视图。  （别名：ClearLog, cls, lc, lclr）
- [DisableLog](#x64dbg-gui-disablelog) — 禁用 Log 视图输出。  （别名：DisableLog, LogDisable）
- [disasm](#x64dbg-gui-disasm) — 在 GUI 中跳转到反汇编视图指定地址。  （别名：disasm, dis, d）
- [dump](#x64dbg-gui-dump) — 在 Dump 视图中显示指定地址内存。  （别名：dump）
- [EnableLog](#x64dbg-gui-enablelog) — 启用 Log 视图输出。  （别名：EnableLog, LogEnable）
- [FoldDisassembly](#x64dbg-gui-folddisassembly) — 折叠/展开反汇编（fold disassembly）。  （别名：FoldDisassembly）
- [graph](#x64dbg-gui-graph) — 打开/刷新图形视图（graph）。  （别名：graph）
- [guiupdatedisable](#x64dbg-gui-guiupdatedisable) — 禁用 GUI 自动刷新（减少卡顿）。  （别名：guiupdatedisable）
- [guiupdateenable](#x64dbg-gui-guiupdateenable) — 启用 GUI 自动刷新。  （别名：guiupdateenable）
- [memmapdump](#x64dbg-gui-memmapdump) — 在 MemMap/Dump 视图中显示指定内存段。  （别名：memmapdump）
- [refadd](#x64dbg-gui-refadd) — 向 References 结果表添加一条记录。  （别名：refadd）
- [refget](#x64dbg-gui-refget) — 从 References 结果表读取一条记录/信息。  （别名：refget）
- [refinit](#x64dbg-gui-refinit) — 初始化 References 结果表。  （别名：refinit）
- [sdump](#x64dbg-gui-sdump) — 在 Dump 视图中显示指定地址（可能为符号/段相关 dump）。  （别名：sdump）
- [setfreezestack](#x64dbg-gui-setfreezestack) — 冻结/解冻 Stack 视图的自动跟随。  （别名：setfreezestack）

### AddFavouriteCommand  <a id="x64dbg-gui-addfavouritecommand"></a>

- **Skill ID**：`x64dbg.gui.addfavouritecommand`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`AddFavouriteCommand`
- **一句话用途**：将某命令添加到 Favorites。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/addfavouritecommand.html`

#### 什么时候用（Use cases）
- 把常用工具/命令加入收藏，或设置快捷键。
- 为团队/脚本预置常用操作入口。
- 折叠反汇编以提升可读性。

#### 语法（Syntax）

`AddFavouriteCommand <command>`

#### 参数详解（Arguments）
- **`command`**（必需）：
  命令名或命令文本（上下文相关）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddFavouriteCommand 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddFavouriteTool  <a id="x64dbg-gui-addfavouritetool"></a>

- **Skill ID**：`x64dbg.gui.addfavouritetool`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`AddFavouriteTool`
- **一句话用途**：将某工具添加到 Favorites。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/addfavouritetool.html`

#### 什么时候用（Use cases）
- 把常用工具/命令加入收藏，或设置快捷键。
- 为团队/脚本预置常用操作入口。
- 折叠反汇编以提升可读性。

#### 语法（Syntax）

`AddFavouriteTool <path_or_name>`

#### 参数详解（Arguments）
- **`path_or_name`**（必需）：
  路径或名称。用于加载插件/脚本等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddFavouriteTool "C:\\temp\\a.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### AddFavouriteToolShortcut  <a id="x64dbg-gui-addfavouritetoolshortcut"></a>

- **Skill ID**：`x64dbg.gui.addfavouritetoolshortcut`
- **分类**：GUI/界面控制 (GUI)
- **命令/别名**：`AddFavouriteToolShortcut`（别名：SetFavouriteToolShortcut）
- **一句话用途**：为某 Favorite 工具设置快捷键。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/addfavouritetoolshortcut.html`

#### 什么时候用（Use cases）
- 把常用工具/命令加入收藏，或设置快捷键。
- 为团队/脚本预置常用操作入口。
- 折叠反汇编以提升可读性。

#### 语法（Syntax）

`AddFavouriteToolShortcut <tool>, <shortcut>`

#### 参数详解（Arguments）
- **`tool`**（必需）：
  工具名称/路径（用于 AddFavouriteTool 等）。
- **`shortcut`**（必需）：
  快捷键描述（如 Ctrl+Alt+X）。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `AddFavouriteToolShortcut "C:\\tools\\ida.exe", "Ctrl+Alt+X"`
- `SetFavouriteToolShortcut "C:\\tools\\ida.exe", "Ctrl+Alt+X"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### ClearLog  <a id="x64dbg-gui-clearlog"></a>

- **Skill ID**：`x64dbg.gui.clearlog`
- **分类**：GUI/界面控制 (GUI)
- **命令/别名**：`ClearLog`（别名：cls, lc, lclr）
- **一句话用途**：清空 Log 视图。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/clearlog.html`

#### 什么时候用（Use cases）
- 启用/禁用日志窗口输出，或清空日志。
- 在大量输出前先 clear，避免噪声。
- 在自动化采样中控制日志量。

#### 语法（Syntax）

`ClearLog`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `ClearLog`
- `lc`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### DisableLog  <a id="x64dbg-gui-disablelog"></a>

- **Skill ID**：`x64dbg.gui.disablelog`
- **分类**：GUI/界面控制 (GUI)
- **命令/别名**：`DisableLog`（别名：LogDisable）
- **一句话用途**：禁用 Log 视图输出。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/disablelog.html`

#### 什么时候用（Use cases）
- 启用/禁用日志窗口输出，或清空日志。
- 在大量输出前先 clear，避免噪声。
- 在自动化采样中控制日志量。

#### 语法（Syntax）

`DisableLog`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `DisableLog`
- `LogDisable`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### disasm  <a id="x64dbg-gui-disasm"></a>

- **Skill ID**：`x64dbg.gui.disasm`
- **分类**：GUI/界面控制 (GUI)
- **命令/别名**：`disasm`（别名：dis, d）
- **一句话用途**：在 GUI 中跳转到反汇编视图指定地址。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/disasm.html`

#### 什么时候用（Use cases）
- 打开/切换到对应 GUI 视图，定位到某地址或刷新显示。
- 在自动化中把结果呈现给人类（例如搜索结果后打开 ref/graph）。
- 在多窗口工作流里快速同步视图位置。

#### 语法（Syntax）

`disasm <addr_or_symbol>`

#### 参数详解（Arguments）
- **`addr_or_symbol`**（必需）：
  地址或符号。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `disasm kernel32.GetProcAddress`
- `d kernel32.GetProcAddress`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### dump  <a id="x64dbg-gui-dump"></a>

- **Skill ID**：`x64dbg.gui.dump`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`dump`
- **一句话用途**：在 Dump 视图中显示指定地址内存。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/dump.html`

#### 什么时候用（Use cases）
- 打开/切换到对应 GUI 视图，定位到某地址或刷新显示。
- 在自动化中把结果呈现给人类（例如搜索结果后打开 ref/graph）。
- 在多窗口工作流里快速同步视图位置。

#### 语法（Syntax）

`dump <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `dump 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### EnableLog  <a id="x64dbg-gui-enablelog"></a>

- **Skill ID**：`x64dbg.gui.enablelog`
- **分类**：GUI/界面控制 (GUI)
- **命令/别名**：`EnableLog`（别名：LogEnable）
- **一句话用途**：启用 Log 视图输出。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/enablelog.html`

#### 什么时候用（Use cases）
- 启用/禁用日志窗口输出，或清空日志。
- 在大量输出前先 clear，避免噪声。
- 在自动化采样中控制日志量。

#### 语法（Syntax）

`EnableLog`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `EnableLog`
- `LogEnable`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### FoldDisassembly  <a id="x64dbg-gui-folddisassembly"></a>

- **Skill ID**：`x64dbg.gui.folddisassembly`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`FoldDisassembly`
- **一句话用途**：折叠/展开反汇编（fold disassembly）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/folddisassembly.html`

#### 什么时候用（Use cases）
- 把常用工具/命令加入收藏，或设置快捷键。
- 为团队/脚本预置常用操作入口。
- 折叠反汇编以提升可读性。

#### 语法（Syntax）

`FoldDisassembly <addr_or_range>`

#### 参数详解（Arguments）
- **`addr_or_range`**（必需）：
  地址或范围。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `FoldDisassembly 401000, 402000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### graph  <a id="x64dbg-gui-graph"></a>

- **Skill ID**：`x64dbg.gui.graph`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`graph`
- **一句话用途**：打开/刷新图形视图（graph）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/graph.html`

#### 什么时候用（Use cases）
- 打开/切换到对应 GUI 视图，定位到某地址或刷新显示。
- 在自动化中把结果呈现给人类（例如搜索结果后打开 ref/graph）。
- 在多窗口工作流里快速同步视图位置。

#### 语法（Syntax）

`graph <addr_or_func>`

#### 参数详解（Arguments）
- **`addr_or_func`**（必需）：
  地址或函数。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `graph 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### guiupdatedisable  <a id="x64dbg-gui-guiupdatedisable"></a>

- **Skill ID**：`x64dbg.gui.guiupdatedisable`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`guiupdatedisable`
- **一句话用途**：禁用 GUI 自动刷新（减少卡顿）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/guiupdatedisable.html`

#### 什么时候用（Use cases）
- 批量命令执行前关闭 GUI 更新，提升性能，避免闪烁。
- 批量修改/标注后再打开 GUI 更新一次刷新。
- 脚本执行时减少 UI 事件干扰。

#### 语法（Syntax）

`guiupdatedisable`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `guiupdatedisable`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### guiupdateenable  <a id="x64dbg-gui-guiupdateenable"></a>

- **Skill ID**：`x64dbg.gui.guiupdateenable`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`guiupdateenable`
- **一句话用途**：启用 GUI 自动刷新。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/guiupdateenable.html`

#### 什么时候用（Use cases）
- 批量命令执行前关闭 GUI 更新，提升性能，避免闪烁。
- 批量修改/标注后再打开 GUI 更新一次刷新。
- 脚本执行时减少 UI 事件干扰。

#### 语法（Syntax）

`guiupdateenable`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `guiupdateenable`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### memmapdump  <a id="x64dbg-gui-memmapdump"></a>

- **Skill ID**：`x64dbg.gui.memmapdump`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`memmapdump`
- **一句话用途**：在 MemMap/Dump 视图中显示指定内存段。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/memmapdump.html`

#### 什么时候用（Use cases）
- 打开/切换到对应 GUI 视图，定位到某地址或刷新显示。
- 在自动化中把结果呈现给人类（例如搜索结果后打开 ref/graph）。
- 在多窗口工作流里快速同步视图位置。

#### 语法（Syntax）

`memmapdump <addr_or_region>`

#### 参数详解（Arguments）
- **`addr_or_region`**（必需）：
  地址或内存区域。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `memmapdump 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### refadd  <a id="x64dbg-gui-refadd"></a>

- **Skill ID**：`x64dbg.gui.refadd`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`refadd`
- **一句话用途**：向 References 结果表添加一条记录。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/refadd.html`

#### 什么时候用（Use cases）
- 初始化引用视图并向其中添加行（refinit/refadd）。
- 从脚本读取引用视图条目（refget）。
- 把搜索/分析结果输出成可点击的列表。

#### 语法（Syntax）

`refadd <addr>, <text>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`text`**（必需）：
  通用文本字符串。建议用引号包裹。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `refadd 401000, "text"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### refget  <a id="x64dbg-gui-refget"></a>

- **Skill ID**：`x64dbg.gui.refget`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`refget`
- **一句话用途**：从 References 结果表读取一条记录/信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/refget.html`

#### 什么时候用（Use cases）
- 初始化引用视图并向其中添加行（refinit/refadd）。
- 从脚本读取引用视图条目（refget）。
- 把搜索/分析结果输出成可点击的列表。

#### 语法（Syntax）

`refget <index>`

#### 参数详解（Arguments）
- **`index`**（必需）：
  索引（一般从 0 或 1 开始，取决于列表）。建议先用 list/get 命令确认。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 读取类命令可能会把读取到的值写到 $result 或返回字符串到特定变量（依实现）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `refget 0`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### refinit  <a id="x64dbg-gui-refinit"></a>

- **Skill ID**：`x64dbg.gui.refinit`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`refinit`
- **一句话用途**：初始化 References 结果表。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/refinit.html`

#### 什么时候用（Use cases）
- 初始化引用视图并向其中添加行（refinit/refadd）。
- 从脚本读取引用视图条目（refget）。
- 把搜索/分析结果输出成可点击的列表。

#### 语法（Syntax）

`refinit`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `refinit`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### sdump  <a id="x64dbg-gui-sdump"></a>

- **Skill ID**：`x64dbg.gui.sdump`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`sdump`
- **一句话用途**：在 Dump 视图中显示指定地址（可能为符号/段相关 dump）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/sdump.html`

#### 什么时候用（Use cases）
- 打开/切换到对应 GUI 视图，定位到某地址或刷新显示。
- 在自动化中把结果呈现给人类（例如搜索结果后打开 ref/graph）。
- 在多窗口工作流里快速同步视图位置。

#### 语法（Syntax）

`sdump <addr>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `sdump 401000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setfreezestack  <a id="x64dbg-gui-setfreezestack"></a>

- **Skill ID**：`x64dbg.gui.setfreezestack`
- **分类**：GUI/界面控制 (GUI)
- **命令**：`setfreezestack`
- **一句话用途**：冻结/解冻 Stack 视图的自动跟随。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/gui/setfreezestack.html`

#### 什么时候用（Use cases）
- GUI 视图控制、引用视图与日志窗口控制。
- 让脚本输出可视化、可导航。
- 自动化批处理时注意 GUI 更新开销。

#### 语法（Syntax）

`setfreezestack <0|1>`

#### 参数详解（Arguments）
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setfreezestack 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）


## 杂项 (Miscellaneous)

杂项命令包含：
- 当前目录切换、sleep；
- HideDebugger（反反调试辅助）；
- loadlib（加载 DLL）、asm（汇编相关）、gpa（GetProcAddress 辅助）；
- JIT 调试器配置（setjit/getjit/...）；
- 进程命令行查询/修改；
- 指令助记符帮助、config 配置等。

### 本分类命令索引

- [asm](#x64dbg-miscellaneous-asm) — 将汇编文本汇编并写入内存（assemble）。  （别名：asm）
- [chd](#x64dbg-miscellaneous-chd) — 切换工作目录（change directory）。  （别名：chd）
- [config](#x64dbg-miscellaneous-config) — 读取/修改 x64dbg 配置项。  （别名：config）
- [getcommandline](#x64dbg-miscellaneous-getcommandline) — 获取当前调试目标的命令行。  （别名：getcommandline, getcmdline）
- [getjit](#x64dbg-miscellaneous-getjit) — 获取 JIT 调试器配置。  （别名：getjit, jitget）
- [getjitauto](#x64dbg-miscellaneous-getjitauto) — 获取 JIT 自动附加配置。  （别名：getjitauto, jitgetauto）
- [gpa](#x64dbg-miscellaneous-gpa) — GetProcAddress：解析模块导出地址。  （别名：gpa）
- [HideDebugger](#x64dbg-miscellaneous-hidedebugger) — 启用/关闭反调试隐藏选项（HideDebugger）。  （别名：HideDebugger, dbh, hide）
- [loadlib](#x64dbg-miscellaneous-loadlib) — 在被调试进程中加载 DLL（LoadLibrary）。  （别名：loadlib）
- [mnemonicbrief](#x64dbg-miscellaneous-mnemonicbrief) — 显示某条指令助记符的简要说明。  （别名：mnemonicbrief）
- [mnemonichelp](#x64dbg-miscellaneous-mnemonichelp) — 显示某条指令助记符的帮助信息。  （别名：mnemonichelp）
- [setcommandline](#x64dbg-miscellaneous-setcommandline) — 设置调试目标的命令行。  （别名：setcommandline, setcmdline）
- [setjit](#x64dbg-miscellaneous-setjit) — 设置 JIT 调试器相关配置。  （别名：setjit, jitset）
- [setjitauto](#x64dbg-miscellaneous-setjitauto) — 设置 JIT 自动附加配置。  （别名：setjitauto, jitsetauto）
- [zzz](#x64dbg-miscellaneous-zzz) — 睡眠/延迟一段时间（毫秒）。  （别名：zzz, doSleep）

### asm  <a id="x64dbg-miscellaneous-asm"></a>

- **Skill ID**：`x64dbg.miscellaneous.asm`
- **分类**：杂项 (Miscellaneous)
- **命令**：`asm`
- **一句话用途**：将汇编文本汇编并写入内存（assemble）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/asm.html`

#### 什么时候用（Use cases）
- asm：把汇编文本转换/写入（用于 patch 或生成机器码，具体依实现）。
- gpa：辅助获取 GetProcAddress 相关信息（根据实现可能返回地址）。
- 在脚本化 patch/注入流程中作为基础积木。

#### 语法（Syntax）

`asm <addr>, <asm_text>`

#### 参数详解（Arguments）
- **`addr`**（必需）：
  地址表达式（Virtual Address/VA）。可为十六进制常量、符号（module+offset）、寄存器值、表达式等。
- **`asm_text`**（必需）：
  汇编文本（x86/x64 指令字符串）。用于 asm/patch 等。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `asm 401000, "mov eax, 1"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### chd  <a id="x64dbg-miscellaneous-chd"></a>

- **Skill ID**：`x64dbg.miscellaneous.chd`
- **分类**：杂项 (Miscellaneous)
- **命令**：`chd`
- **一句话用途**：切换工作目录（change directory）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/chd.html`

#### 什么时候用（Use cases）
- 切换当前工作目录，影响后续相对路径解析（脚本/导出文件）。
- 在自动化前设置输出目录，避免文件散落。
- 与 savedata/minidump/scriptload 等命令的相对路径配合。

#### 语法（Syntax）

`chd <path>`

#### 参数详解（Arguments）
- **`path`**（必需）：
  路径参数。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `chd "C:\\temp"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### config  <a id="x64dbg-miscellaneous-config"></a>

- **Skill ID**：`x64dbg.miscellaneous.config`
- **分类**：杂项 (Miscellaneous)
- **命令**：`config`
- **一句话用途**：读取/修改 x64dbg 配置项。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/config.html`

#### 什么时候用（Use cases）
- 读取/修改 x64dbg 配置项（key/value）。
- 在自动化环境中统一设置（例如 UI、分析、日志相关选项）。
- 把环境配置脚本化，提高可复现性。

#### 语法（Syntax）

`config <key>[, <value>]`

#### 参数详解（Arguments）
- **`key`**（必需）：
  配置键名。
- **`value`**（可选）：
  通用数值参数。可以是常量、表达式计算结果等。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `config "SomeConfigKey"`
- `config "SomeConfigKey", 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### getcommandline  <a id="x64dbg-miscellaneous-getcommandline"></a>

- **Skill ID**：`x64dbg.miscellaneous.getcommandline`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`getcommandline`（别名：getcmdline）
- **一句话用途**：获取当前调试目标的命令行。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/getcommandline.html`

#### 什么时候用（Use cases）
- 读取/修改目标进程的命令行参数（或调试器的相关配置，取决于实现）。
- 在启动后修正参数以复现不同运行路径。
- 在自动化中记录当前命令行用于复现。

#### 语法（Syntax）

`getcommandline`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 读取类命令可能会把读取到的值写到 $result 或返回字符串到特定变量（依实现）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `getcommandline`
- `getcmdline`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### getjit  <a id="x64dbg-miscellaneous-getjit"></a>

- **Skill ID**：`x64dbg.miscellaneous.getjit`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`getjit`（别名：jitget）
- **一句话用途**：获取 JIT 调试器配置。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/getjit.html`

#### 什么时候用（Use cases）
- 读取/设置系统 JIT 调试器相关配置。
- 在逆向环境中把 x64dbg 注册为 JIT 调试器。
- 自动化脚本中检测当前 JIT 状态并修正。

#### 语法（Syntax）

`getjit`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。
- 读取类命令可能会把读取到的值写到 $result 或返回字符串到特定变量（依实现）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `getjit`
- `jitget`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### getjitauto  <a id="x64dbg-miscellaneous-getjitauto"></a>

- **Skill ID**：`x64dbg.miscellaneous.getjitauto`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`getjitauto`（别名：jitgetauto）
- **一句话用途**：获取 JIT 自动附加配置。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/getjitauto.html`

#### 什么时候用（Use cases）
- 读取/设置系统 JIT 调试器相关配置。
- 在逆向环境中把 x64dbg 注册为 JIT 调试器。
- 自动化脚本中检测当前 JIT 状态并修正。

#### 语法（Syntax）

`getjitauto`

#### 参数详解（Arguments）
- （无参数）

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `getjitauto`
- `jitgetauto`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### gpa  <a id="x64dbg-miscellaneous-gpa"></a>

- **Skill ID**：`x64dbg.miscellaneous.gpa`
- **分类**：杂项 (Miscellaneous)
- **命令**：`gpa`
- **一句话用途**：GetProcAddress：解析模块导出地址。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/gpa.html`

#### 什么时候用（Use cases）
- asm：把汇编文本转换/写入（用于 patch 或生成机器码，具体依实现）。
- gpa：辅助获取 GetProcAddress 相关信息（根据实现可能返回地址）。
- 在脚本化 patch/注入流程中作为基础积木。

#### 语法（Syntax）

`gpa <module>, <proc_name>`

#### 参数详解（Arguments）
- **`module`**（必需）：
  模块名（如 user32.dll）。
- **`proc_name`**（必需）：
  进程名。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `gpa kernel32.dll, "notepad.exe"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### HideDebugger  <a id="x64dbg-miscellaneous-hidedebugger"></a>

- **Skill ID**：`x64dbg.miscellaneous.hidedebugger`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`HideDebugger`（别名：dbh, hide）
- **一句话用途**：启用/关闭反调试隐藏选项（HideDebugger）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/hidedebugger.html`

#### 什么时候用（Use cases）
- 在遇到反调试检测时尝试隐藏调试器特征（反反调试）。
- 在启动/附加后立即调用，降低被检测概率。
- 与断点/patch 组合绕过检测逻辑。

#### 语法（Syntax）

`HideDebugger <0|1>`

#### 参数详解（Arguments）
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 隐藏调试器可能改变系统/调试器行为；在某些目标上可能引入兼容性问题。

#### 示例（Examples）
- `HideDebugger 1`
- `dbh 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### loadlib  <a id="x64dbg-miscellaneous-loadlib"></a>

- **Skill ID**：`x64dbg.miscellaneous.loadlib`
- **分类**：杂项 (Miscellaneous)
- **命令**：`loadlib`
- **一句话用途**：在被调试进程中加载 DLL（LoadLibrary）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/loadlib.html`

#### 什么时候用（Use cases）
- 在目标进程中加载某个 DLL（LoadLibrary 语义）。
- 触发 DLLMain/初始化逻辑以便调试。
- 与 Librarian 断点联动：加载即断。

#### 语法（Syntax）

`loadlib <dll_path>`

#### 参数详解（Arguments）
- **`dll_path`**（必需）：
  DLL 路径。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `loadlib "C:\\Windows\\System32\\user32.dll"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### mnemonicbrief  <a id="x64dbg-miscellaneous-mnemonicbrief"></a>

- **Skill ID**：`x64dbg.miscellaneous.mnemonicbrief`
- **分类**：杂项 (Miscellaneous)
- **命令**：`mnemonicbrief`
- **一句话用途**：显示某条指令助记符的简要说明。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/mnemonicbrief.html`

#### 什么时候用（Use cases）
- 查询某个指令助记符的帮助/简要说明。
- 在脚本生成汇编/patch 前确认指令含义。
- 作为交互式学习/查阅工具。

#### 语法（Syntax）

`mnemonicbrief <mnemonic>`

#### 参数详解（Arguments）
- **`mnemonic`**（必需）：
  通用参数（请结合官方页面确认具体格式）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `mnemonicbrief 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### mnemonichelp  <a id="x64dbg-miscellaneous-mnemonichelp"></a>

- **Skill ID**：`x64dbg.miscellaneous.mnemonichelp`
- **分类**：杂项 (Miscellaneous)
- **命令**：`mnemonichelp`
- **一句话用途**：显示某条指令助记符的帮助信息。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/mnemonichelp.html`

#### 什么时候用（Use cases）
- 查询某个指令助记符的帮助/简要说明。
- 在脚本生成汇编/patch 前确认指令含义。
- 作为交互式学习/查阅工具。

#### 语法（Syntax）

`mnemonichelp <mnemonic>`

#### 参数详解（Arguments）
- **`mnemonic`**（必需）：
  通用参数（请结合官方页面确认具体格式）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `mnemonichelp 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setcommandline  <a id="x64dbg-miscellaneous-setcommandline"></a>

- **Skill ID**：`x64dbg.miscellaneous.setcommandline`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`setcommandline`（别名：setcmdline）
- **一句话用途**：设置调试目标的命令行。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/setcommandline.html`

#### 什么时候用（Use cases）
- 读取/修改目标进程的命令行参数（或调试器的相关配置，取决于实现）。
- 在启动后修正参数以复现不同运行路径。
- 在自动化中记录当前命令行用于复现。

#### 语法（Syntax）

`setcommandline <cmdline>`

#### 参数详解（Arguments）
- **`cmdline`**（必需）：
  传给目标进程的命令行参数字符串（不含可执行文件本身）。建议整段用引号包裹，内部引号做好转义。

#### 执行上下文 / 前置条件（Preconditions）
- 需要存在已加载的调试会话（已启动/已附加），否则命令可能无效。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setcommandline "-arg1 -arg2"`
- `setcmdline "-arg1 -arg2"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setjit  <a id="x64dbg-miscellaneous-setjit"></a>

- **Skill ID**：`x64dbg.miscellaneous.setjit`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`setjit`（别名：jitset）
- **一句话用途**：设置 JIT 调试器相关配置。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/setjit.html`

#### 什么时候用（Use cases）
- 读取/设置系统 JIT 调试器相关配置。
- 在逆向环境中把 x64dbg 注册为 JIT 调试器。
- 自动化脚本中检测当前 JIT 状态并修正。

#### 语法（Syntax）

`setjit <path_or_options>`

#### 参数详解（Arguments）
- **`path_or_options`**（必需）：
  路径或选项。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setjit "C:\\temp\\a.bin"`
- `jitset "C:\\temp\\a.bin"`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### setjitauto  <a id="x64dbg-miscellaneous-setjitauto"></a>

- **Skill ID**：`x64dbg.miscellaneous.setjitauto`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`setjitauto`（别名：jitsetauto）
- **一句话用途**：设置 JIT 自动附加配置。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/setjitauto.html`

#### 什么时候用（Use cases）
- 读取/设置系统 JIT 调试器相关配置。
- 在逆向环境中把 x64dbg 注册为 JIT 调试器。
- 自动化脚本中检测当前 JIT 状态并修正。

#### 语法（Syntax）

`setjitauto <0|1>`

#### 参数详解（Arguments）
- **`0|1`**（必需）：
  布尔开关：0=关闭，1=开启。Agent 应只生成 0/1，避免 true/false 兼容性问题。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `setjitauto 1`
- `jitsetauto 1`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

### zzz  <a id="x64dbg-miscellaneous-zzz"></a>

- **Skill ID**：`x64dbg.miscellaneous.zzz`
- **分类**：杂项 (Miscellaneous)
- **命令/别名**：`zzz`（别名：doSleep）
- **一句话用途**：睡眠/延迟一段时间（毫秒）。
- **官方页面（推断链接）**：`https://help.x64dbg.com/en/latest/commands/miscellaneous/zzz.html`

#### 什么时候用（Use cases）
- 在脚本中主动等待一段时间（ms），用于同步外部事件。
- 配合 attach/模块加载等场景等待目标进入稳定状态。
- 避免 busy loop，降低 CPU 占用。

#### 语法（Syntax）

`zzz <ms>`

#### 参数详解（Arguments）
- **`ms`**（必需）：
  毫秒数（sleep/pause 等）。

#### 执行上下文 / 前置条件（Preconditions）
- 一般无特殊前置条件，但建议在暂停态执行以保证可重复性。

#### 返回与可观测输出（Returns / Observable effects）
- $result：大多数命令会把成功/失败写入 $result（常见：成功=1，失败=0）。

#### 副作用与风险（Side effects / Risks）
- 低到中等风险：主要取决于参数是否正确与当前是否处于暂停态。

#### 示例（Examples）
- `zzz 1000`
- `doSleep 1000`

#### Agent 封装建议（Skill wrapper tips）

建议把本命令封装成一个“原子技能”，并统一做：参数校验、状态校验、执行后采样。最小建议：
- `validate_args()`：检查地址/大小/布尔开关/路径是否合理（能否解析、是否为空、是否越界等）。
- `ensure_paused_if_writing()`：若命令可能写内存/寄存器/DB，优先要求暂停态。
- `capture_before_after()`：对写操作记录 before/after（至少记录目标地址/寄存器与 CIP）。
- `parse_result()`：从 `$result` 或 GUI 结果列表中提取结构化输出。

（如果你需要我把这些 wrapper 输出成可直接喂给某个 Agent 框架的 YAML/JSON schema，我也可以基于本条目继续生成。）

