---
name: defect-detection
description: 检测嵌入式C代码中的中断并发缺陷（竞态条件/原子性违反/运行时缺陷等）。当用户提供嵌入式C项目路径并要求检测中断相关缺陷时使用。
---

# 嵌入式中断并发缺陷检测

**MCP 工具**（server "ice-interrupt-analyzer"）：`interrupt_analyze` / `interrupt_analyze_merge` / `interrupt_model_build` / `interrupt_model_patch` / `interrupt_z3_verify` / `interrupt_export_report`

## 配置约定

- 将 `detection_modes` 的真值来源视为**仓库根目录 `ice_config.yml`**。
- `interrupt_model_build()` 与 `interrupt_z3_verify()` 会在 MCP 入口层自动读取根配置，并把 `detection_modes` 传入 `DefectDetector(...)`。
- 当用户没有明确要求只跑某一类缺陷时，按根配置执行，不要在 skill 层额外维护一份 mode 列表。
- 后续新增 defect mode 时，优先沿这条链接入：`ice_config.yml -> 入口层 -> DefectDetector`。

---

## Stage 1：中断信息提取

**知识库已知架构**（真值来源：`ice_core/knowledge_base/interrupt_info.yml` 中的 `architectures` 与各类 `*_patterns`）均已配置静态正则：**ARM**（Cortex-M / CMSIS / STM32 HAL）、**AVR**、**MSP430**、**RISC-V**、**Generic**（含 Keil C51 / POSIX-like / 通用 `_ISR` 等）、**TestSuite**（SVP / ICDBench 等 `_isr` / `enable_isr` 约定）。凡源码形态可被上述模式覆盖的项目，**一律优先相信静态分析结果**，Stage 1 默认走静态路径；OpenCode 仅在静态明显不足时做**有源码证据的补充**，不能把不确定信息补成“看起来合理”的确定值。

**默认（知识库覆盖的已知架构）** → 直接静态分析：
```
interrupt_analyze(project_path=<path>, mode="static")
```

若 `interrupt_info.yml` 中已有对应 ISR / 开关 / 优先级模式命中，**不要**为了“再确认一遍”而改走 `agent`；静态产物即为该阶段的主结论。

**仅当**出现例如：静态结果中 `interrupt_functions` 为空或与源码明显不符、ISR 使用知识库未覆盖的自定义约定、或用户明确说明为非表内架构时，再进入 agent 补全路径：
```
interrupt_analyze(project_path=<path>, mode="agent")
```

在 `mode="agent"` 下：

1. 调用后读取**同目录下已由静态分析写出的结果**与 `supplement_context`（函数列表、开关操作、可补充位置）。
2. 仅当源码中存在**可引用的直接证据**时，才补充以下信息：
   - ISR / interrupt function 身份；
   - `priority`（仅在源码能确定数值时填写）；
   - `switch_targets`（仅在源码能确定具体目标 ISR 时填写）。
3. 证据必须来自实际源代码，例如：
   - 中断向量表、IRQ 注册表、回调绑定点；
   - `NVIC_SetPriority(...)`、平台等价 API、寄存器/宏定义展开后的明确赋值；
   - `EnableIRQ(...)` / `DisableIRQ(...)` / 掩码操作附近能明确对应到 ISR 的代码。
4. 下列情况一律**保持未知，不要猜测**：
   - 只能看出“像 ISR”但没有注册/向量/调用证据；
   - 只能拿到 `priority_raw` / 宏 / 表达式，但无法从源码确定最终数值；
   - 只知道发生了开关中断，但无法确定作用到哪个 ISR；
   - 只能从经验推断“哪个数字优先级更高”。
5. 区分**原始优先级值**与**架构相关的排序语义**：
   - 从源码/API 读到的只是 raw priority value；
   - 不要给出“数值大小与优先级高低如何对应”的跨架构通用结论，除非源码/平台定义在当前项目中明确给出；
   - 如果只能确认原始值，补 raw 数值即可，不要额外臆断其高低顺序语义。
6. 处理 ARM / STM32 项目时，优先按下面的证据链补全，不要跳步：
   - 先查 `NVIC_SetPriority(...)` / `HAL_NVIC_SetPriority(...)` / `NVIC_EnableIRQ(...)` / `HAL_NVIC_EnableIRQ(...)`；
   - 再沿 `#define XXX_IRQn YYY_IRQn`、`#define XXX_IRQHandler YYY_IRQHandler` 做 IRQ / handler 别名链展开；
   - 若 `priority` 是宏，继续展开一层到宏体；若宏体是 `NVIC_EncodePriority(..., preempt, sub)`，只把 `preempt` 当作可落盘的 raw priority 数值来源；
   - 若最终仍只能得到表达式而不是确定数值，则保留 `priority_raw` / 宏名，不要伪造确定整数。
7. 对 OpenCode 的补全范围做硬约束：
   - 只能补 `supplement_context` 中已有函数/开关条目；
   - 不能新造 ISR 名、不能把 callback 猜成 ISR、不能把 `*_IRQn` 自动假定映射到同名 `*IRQHandler`，除非当前项目源码里能找到该映射证据；
   - 对 switch target 只允许返回源码直接可证实的 ISR 列表。
8. 构造 `agent_patch` 时只做**加法式补充**，不要盲目改写静态结果；无证据字段宁可留空。

补丁格式：
```
interrupt_analyze_merge(project_path=<path>, agent_patch={
  "interrupt_functions": [{"name": "<函数名>", "priority": 1}, ...],
  "main_functions": ["<主函数名>"],
  "switch_targets": [{"line": <行号>, "targets": ["<ISR名>"]}]
})
```
> `name` / `line` 必须与 `supplement_context` 中的值完全一致；未列出的函数自动归为 regular。
> 仅提交有源码证据支持的条目；不要为“补全完整性”而虚构 ISR、priority 或 switch target。

---

## Stage 2：路径枚举 + Z3 建模

```
interrupt_model_build(project_path=<path>)
```

返回 `needs_domain_supplement`：
- `false` 且 `valid_triplets_count > 0` → 模型完备，进入 Stage 3
- `false` 且 `valid_triplets_count == 0` → 当前检测模式下**无有效三元组**，直接进入 Stage 4，**不要**回退到重新分析或 agent 模式，也不要在 report.yml 之外另行宣称"未发现缺陷"
- `true` → 需补全变量值域，读取 `agent_input.expressions_to_analyze`，分析每项的取值范围构造并调用：

```
interrupt_model_patch(project_path=<path>, domain_fixes=[
  {"aggregation_id": "<来自agent_input的id>", "domain": {"type": "range", "min": <下界>, "max": <上界>, "inclusive_max": <true|false>, "is_determined": true, "source": "<推断依据>"}},
  {"aggregation_id": "<来自agent_input的id>", "domain": {"type": "single_value", "value": <具体值>, "is_determined": true, "source": "<推断依据>"}}
])
```
> `aggregation_id` 必须原样使用 `agent_input.expressions_to_analyze[*].aggregation_id`，不可自行构造。`min`/`max`/`value` 根据对表达式的语义分析确定，每个表达式独立推断。
> `current_suggestion` 非 null 时可直接采纳；外部 I/O / 硬件寄存器无法推断时跳过该项。

---

## Stage 3：Z3 验证 + 缺陷报告

```
interrupt_z3_verify(project_path=<path>)
```

返回：
- `defect_count`：Z3 为 `sat` 的**模型条数**（`z3_models` 下一文件通常对应一条），同一源码「行三元组 + 共享变量 + 模式」可能被多个嵌入路径重复验证，**数值会远大于独立逻辑缺陷个数**。
- `defects`：上述每一条的摘要（`triplet_id` / `shared_variable` / `violation_pattern` / 三处行号）。
- `defect_count_unique_locations` / `defects_unique_locations`：按「行三元组 + `shared_variable` + `violation_pattern`」去重后的列表；**Stage 4 智能体审查、向用户口述结论、对照源码时只看这里即可**，不要对 `defects` 里每一条重复做深度分析。
- `z3_summary`：验证统计（`sat` / `unsat` / `not_ready` / `total`）。

> **注意**：`not_ready` 待检测模式的元组中表示仍有未确定值域（`interrupt_model_patch` 未覆盖），会被保留为潜在缺陷。需消除补充对应 `aggregation_id` 的值域后重新调用 `interrupt_model_patch` 再验证。

## Stage 4：智能体最终审查与报告导出（必须执行）

在拿到 Z3 验证结果之后，以 **`defects_unique_locations`（或等价地：按三行号 + 变量 + 模式去重后的集合）** 为审查单元做**智能体最终审查**；`defects` 全量仅作溯源（不同 `triplet_id`、同一路径）：
1. **尊重工具结论**：不要轻易否定 Z3 引擎等形式化工具给出的结论。默认情况下，应认可 Z3 给出的验证结果。
2. **粗略审查**：先快速、轻量级地扫视缺陷相关的三元组和源码上下文，寻找极明显的分析盲区。
3. **细致分析（仅限存在明显误报倾向）**：只有在发现**明确的误报倾向**时，才去进行细致的代码推演并尝试否定工具结论。如果判定为误报（`false_positive`），必须在最终结果中给出具体的排除理由（`reason`/`evidence`）。

审查完成后，把包含审查结果的报告文件写入 `project_path/improved_interrupt_analysis/report.yml`（或用户指定目录）。

调用：

```
interrupt_export_report(
  project_path=<path>,
  defects=<必须传 Stage 3 的 defects_unique_locations 原样列表>,
  false_positive_results=[
    {"triplet_id": "triplet_1", "is_false_positive": true, "reason": "存在有效保护条件，危险状态不可达"}
  ],
  evidence="智能体最终证据描述",
  meta={"dataset": "ICDBench", "subset": "AV", "version": "v2"}
)
```

导出的 `report.yml` 结构固定为：

```yaml
case_id: svp_simple_005
defect_type: AV
summary: 检测结果保留 1 个 AV 缺陷三元组。
evidence: 智能体过滤后保留三元组 (32-46-40)。
triplets:
  AV:
    - (32-46-40) | (main.c, main.c, main.c) | global_var1
agent_result_analyze:
  - triplet: (32-46-40)
    shared_variable: global_var1
    defect_type: AV
    result: keep
    evidence: 路径可达，且满足缺陷触发条件，保留为真实缺陷。
  - triplet: (32-46-38)
    shared_variable: global_var1
    defect_type: AV
    result: false_positive
    evidence: 路径约束不可同时满足，候选不成立。
```

规则（稳定输出，必须遵守）：
- `defects` 只传 `defects_unique_locations`，**不要**手写 `triplet` 文本、`op1_file/op3_file`、自定义 `defect_type`。
- 若 Stage 3 返回 `defect_count_unique_locations > 0`，则 `report.yml.triplets` 与 `agent_result_analyze` 不得为空。
- `triplets` 仅保留最终 `keep` 项，单行格式固定：`(行1-行2-行3) | (op1文件, interrupt文件, op3文件) | <shared_variable>`。
- `agent_result_analyze` 与 `defects_unique_locations` 一一对应，每个逻辑三元组只出现一次；字段仅 `triplet/shared_variable/defect_type/result/evidence`。
- 对用户回复可简述，但最终以 `report.yml` 为准。


