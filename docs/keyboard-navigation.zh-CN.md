# 键盘与系统焦点导航探索

[English](keyboard-navigation.md) · 简体中文

后续实现：候选 `0.5.0-mcp` 已实现无需新增手机配置的功能子集，见[当前工具](hid-tools.zh-CN.md)。本文保留此前研究快照；真机行为仍未验证。

日期：2026-10-14。源码基线：`c794813f960b139089a1d4c72106e5c46c265b61`，固件 `0.4.0-mcp`。

状态：完成源码与官方文档检查；以下实验与接口扩展均未在设备运行，也未实现。本文不改变当前 MCP、HID 描述符、输入时序或控制授权。

## 结论

当前固件已经能发送 Tab、Shift+Tab、Space 和 Command+普通键，因此第一轮 Full Keyboard Access（全键盘控制）实验不需要修改固件或 Anan。iPhone 需由用户启用全键盘控制。Anan 通过已有 `phone.observe` 判断截图中的焦点高亮，再通过已有 MCP `key` 操作。

只改固件可以改善工具易用性、减少已知连续按键的调用次数、提供实验用的可配置按键节奏；不能让 ESP32 得知当前控件或自动判断是否操作成功。推荐按“系统快捷键 → 附近焦点导航 → 坐标操作”的组合探索，而不是逐个 Tab 扫描整页。

## 当前键盘究竟有多简单

| 层次 | 当前实现 | 实际含义 |
| --- | --- | --- |
| USB 描述符 | `main/usb.c:21` 使用 TinyUSB 标准键盘报告，report ID 2 | 有 8 位修饰键和 6 个普通键槽；不是只支持输入字母 |
| 底层发送 | `bridge_key()`，`main/usb.c:69`，`keys[6] = {key}` | 当前只填一个普通键槽，可同时附带多个 Ctrl/Shift/Alt/GUI 修饰位 |
| 动作 | `stroke()`，`main/actions.c:86` | 按下、约 12ms 等待、全部释放、约 12ms 等待；实际还有 USB 报告等待与任务调度 |
| MCP | `key` 接收数字 `keycode` 和 `modifiers` | 能表达 Tab、Shift+Tab、Command+Space；对模型不如按键名称易用 |
| 文本 | `type_ascii` 转换 US 布局字符并逐个 `stroke()` | 有界 ASCII，不是 Unicode 注入；焦点、输入法和系统快捷键会影响结果 |

修饰位：左 Ctrl=1、Shift=2、Alt=4、GUI=8；右侧对应 16/32/64/128。在通常的 iPhone 映射中 GUI 对应 Command，但用户可以重映射修饰键。

当前每次动作会释放按键。连续两次 `key` 不能实现“按住 Tab 再按 H”这种两个普通键的同时组合。描述符已有 6 键槽，若未来确实需要，可以扩展发送函数而不必立刻换描述符；第一轮无需这项改动。

## 官方支持与必须区分的按键

Apple 官方的全键盘控制页面确认 Tab 向前、Shift+Tab 向后、Space 激活，以及可见焦点高亮；也允许修改命令绑定与高亮外观。另一个页面确认 Command+Space 打开系统搜索。[全键盘控制](https://support.apple.com/en-kw/guide/iphone/ipha4375873f/ios)、[键盘快捷键](https://support.apple.com/en-kw/guide/iphone/iph3da414515/ios)。

下表都是**现有 MCP 工具名 `key` 的参数**，不是新的工具，也不表示已在 StopWatch 上验证。

| 操作 | 参数 | 前提 |
| --- | --- | --- |
| 下一个焦点项目 | `{"keycode":43,"modifiers":0}` | 全键盘控制开启，使用默认绑定 |
| 上一个焦点项目 | `{"keycode":43,"modifiers":2}` | 同上 |
| 激活所选项目 | `{"keycode":44,"modifiers":0}` | 确认高亮目标；输入上下文中不可盲用 Space |
| 系统搜索 | `{"keycode":44,"modifiers":8}` | 标准 Command 映射；需检查全键盘控制是否改绑/截获 |
| Enter / Escape | `{"keycode":40,"modifiers":0}` / `{"keycode":41,"modifiers":0}` | 动作由当前 App、控件与系统绑定决定 |
| 右 / 左 / 下 / 上 | `keycode` 为 79 / 80 / 81 / 82，`modifiers` 为 0 | 可能移动焦点、光标或控件数值，不概括为统一的焦点导航 |

当前查到的 Apple 全键盘控制文档还列出 Fn+H、Fn+方向上、Fn+C、Fn+N 等系统命令。**现有 8 位修饰字段没有 Fn**；不能拿 Ctrl/Alt/GUI 的某个位冒充 Fn。Tab+H 也不是可由当前单个普通键工具表达的组合。可先在手机的 Commands 中选择能由现有报告发送的自定义绑定，但映射必须明确，不能在固件中假定用户已配置。

## 为什么可能更准确，为什么也可能更慢

坐标点击要求视觉模型估计位置；确认高亮后按 Space，可以把目标落点交给系统焦点机制。这是待验证的准确率收益，不是已经测出的结果。

快捷键可以缩短多步手势；已知短距离的连续 Tab 可以减少 MCP 往返。然而每次 Tab 都观察和推理、或绕过许多无关元素，会增加时间和模型调用。焦点顺序也不一定等于屏幕阅读顺序。因此优先测试附近按钮、短表单和系统搜索，复杂画布继续保留坐标输入。

固件没有屏幕图像、焦点 ID 或组件树。工具只能报告按键执行，不能返回 `focused_label`、`target_reached`，也不能可靠计数“还需几次 Tab”。没有图像判断的长批次只是盲操作。

## 第一轮实验：使用现有固件

这些是后续具体验收范围，不是本次执行记录。需要当前目标和范围的真机操作授权后才由 Agent 执行；用户也可按此手动实验。

准备：手机已配对 MCP、StopWatch 已物理启用、Anan 屏幕共享已确认；手机保持竖屏解锁。用户在 Settings → Accessibility → Keyboards & Typing 中启用 Full Keyboard Access，并增大/提高高亮的可见性。路径名称可能随版本变化。

记录 Slow Keys、Sticky Keys、修饰键映射和命令改绑情况。第一轮使用未启用 Slow Keys / Sticky Keys 的基线：Slow Keys 会影响短按识别，Sticky Keys 会改变组合键行为；不能把失败直接归因于固件。[Apple 键盘辅助设置](https://support.apple.com/en-kw/guide/iphone/ipha7c3927eb/ios)。

| 实验 | 步骤 | 要判断什么 |
| --- | --- | --- |
| 焦点可见性 | 在 Settings 一个稳定页面 observe → Tab → observe | 截图中是否出现可辨认的高亮；实际按键是否被识别 |
| 前后导航 | Tab 后观察，再 Shift+Tab 后观察 | 该页面能否返回先前焦点；遇到焦点组/循环必须记录 |
| 激活 | 移到已确认的无害控件，再 Space，重新观察 | 操作的是高亮控件还是输入字符；不能只看 HID 返回值 |
| 系统搜索 | Command+Space，观察搜索界面，输入 ASCII 的 `Safari`，检查结果 | 是否减少进入 App 的手势；搜索命中和全键盘控制冲突情况 |
| 输入场景 | 在合成文本内容中测试焦点切换、ASCII 输入、Enter | 字符是否误触快捷键、Space 是否输入/激活、焦点是否变更 |
| 混合模式 | 坐标点击接近目标 → Tab / Shift+Tab → observe | 点击和键盘焦点是否一致；不能假设点击一定设定焦点 |
| 停止 | 操作期间物理停止或中断 USB，观察不再输入 | 保留既有撤销/释放和不自动恢复行为 |

同一设备、同一页面和同一任务下，对比纯坐标与混合输入，固定模型与屏幕共享设置。记录任务完成时间、工具调用数、观察数、误激活数、需要纠正的次数及失败；每条结果区分系统识别、视觉判断和最终业务结果。重复少量试次只能判断可行性，不代表广泛统计结论。

可直接给 Anan 的实验需求：

```text
我已开启全键盘控制。用 StopWatch MCP 的 key 测试当前页面的焦点导航：
Tab 是 keycode=43/modifiers=0，Shift+Tab 是 43/2，Space 是 44/0。
先观察屏幕，发送一次 Tab，再观察并说明高亮目标。
寻找目标时最多试 5 步，每步重新观察；看不清高亮就停止或改用坐标。
确认焦点落在目标上后才能按 Space；不要把导航和激活盲目合并。
```

该提示只是使用已有工具，不修改 Anan。输入实验使用合成内容，并避免有副作用的提交/删除等目标。

## 只改固件的候选进步

以下按优先级排列，是讨论用设计，尚未改代码。新增工具/参数属于 MCP 接口扩展，需先确认设计；不改 Anan 源码。

1. **改善现有 `key` 的工具说明**：公布常用键码、修饰位、全键盘控制前提，以及“观察焦点再激活”。实现和参数不变。这可以帮助模型用起现有能力，但不能保证每个模型自动选择最佳策略。
2. **增加一个命名按键工具**：候选 `keyboard`，使用有限枚举 `tab`、`shift_tab`、`space`、`command_space`、`enter`、`escape`、方向键；旧 `key` 保留兼容。不给 `space` 起无条件“点击目标”的名字，也不预设 Fn 系统命令。
3. **有限重复与独立节奏参数**：候选 `repeat` 1…5，仅导航键可重复，默认 1；候选 `pressMs` 20…100，默认 40，`gapMs` 50…300，默认 100。它们仅作用于新工具，保留旧 `key`/ASCII 的时序。数值是实验起点，必须根据目标设备调整，不是已验证最优值。

候选调用：

```json
{"key":"tab","repeat":2,"pressMs":40,"gapMs":100}
```

一次 MCP 调用完成两次 Tab 后，Anan 再观察；**不附带自动 Space**。只有焦点距离有依据时才用短重复，不允许整个页面无人观察地盲扫。

执行继续走现有单 worker、控制 grant、session ownership、取消和超时检查；每次按下、释放和等待都检查中断条件，结束/失败释放按键，不自动重试。返回只能声明实际完成的按键数量及可能的部分执行，不能声明手机目标已被选中。具体部分执行字段需在实现前定稿。

本次只读检查的 Anan `MCPClientAdapter.tools()` 调用 `listTools`，`MCPRemoteTool` 保存服务提供的描述与 JSON Schema，调用也按远端工具名转发。由此判断，增加简单枚举/整数工具原则上可经现有客户端使用；固件更新后需重新获取工具目录。尚未做实际互操作或模型采用率验证。

## 暂不优先做什么

不先升级 NKRO / 键盘描述符或 Apple 专有 Fn 报告；现有常用组合键不需要这些。也不缩短全部输入延迟、不取消物理授权、不把 Unicode 文本当作 HID 字符串发送。中文输入仍是输入法与文本提交问题，不会因全键盘控制自动解决。

建议先得到“Tab 可见、前后导航、Space 正确激活、Command+Space 可用”的结果。若这些在目标 App 上成立，再实现命名工具和有限重复；若遍历顺序差、截图高亮不清楚，则保留快捷键收益，不把全键盘控制作为默认全局导航策略。

## 源码核查入口

- [USB 描述符与键盘报告](../main/usb.c)：`reports`、`bridge_key`、`bridge_release`。
- [动作与按键时序](../main/actions.c)：`ascii_key`、`stroke`、`execute`、`action_worker`。
- [参数及执行授权](../main/mcp.c)：`parse_input`、`input_alive`、`respond_input`。
- [现有工具定义](../main/mcp_tools.h)：`key` 与 `type_ascii`。
- 键值与修饰位按锁定 TinyUSB `0.21.0~2` 的 `src/class/hid/hid.h` 核对；描述符按 `hid_device.h` 核对。生成组件不提交。
- 独立 Anan 客户端仅只读检查 `apps/ios/Anan/MCP/MCPClientAdapter.swift`、`MCPToolDirectory.swift`、`MCPRuntime.swift`，未修改、未执行；这些不是本固件仓库的文件。
