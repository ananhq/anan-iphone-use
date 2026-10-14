# HID 工具参考

[English](hid-tools.md) · 简体中文

候选 `0.5.1-mcp` 通过 MCP `tools/list` 展示七个工具，合并了 `0.5.0-mcp` 的十三项手势和按键入口。不要求修改 Anan 代码或新增手机绑定。升级后刷新客户端工具目录；缓存的旧定义可能拒绝新参数。手机行为及客户端互操作仍待真机验收。

保留原有 AssistiveTouch、USB Ethernet、配对/导入与物理“开始控制”。全键盘控制可选，固件既不启用也不检测；Tab/方向键依 App 和已有系统设置工作。

候选 `0.7.3-discovery` 仅调整描述，增加打开应用流程、浏览、输入/编辑和绘图的中英文任务词；七个 schema 和执行不变。服务柔性建议在多步规划前、概览尚不可见时浏览整组，见[工具发现说明](tool-discovery.zh-CN.md)。

## 选择操作

| 工具 | 参数 | 用途 |
| --- | --- | --- |
| `device_status` | 无 | USB、授权和输入忙碌状态，不提供截图 |
| `tap` | `x`,`y`：0…1；可选 `count`：1/2（默认1）、`holdMs`：0或100…2000（0）、`intervalMs`：50…300（100），仅 count=2 时可传 | 默认单击；count=2 双击；holdMs>0 长按。拒绝双击加长按 |
| `drag` | `points`：2…32个 `{x,y}`；`durationMs`：100…2000且≥20×（点数−1）；可选 `holdMs`：0…1000（0） | 两点滑动/拖动，多点绘制一笔连续路径；需要时在移动前保持 |
| `press_key` | `key`：下列名称；可选 `repeat`：1…5（1）、`pressMs`：20…100（40）、`gapMs`：50…300（100） | 命名键盘/编辑及系统/媒体按键，仅导航允许重复 |
| `type_ascii` | `text`：1…120字节，US 布局可打印 ASCII 加 TAB/LF | 向当前焦点输入；不支持中文、Unicode或输入法注入 |
| `scroll` | `x`,`y`：0…1；`vertical`：整数−10…10；可选 `horizontal`：−10…10（0）、`repeat`：1…5（1）、`gapMs`：50…300（100） | 在目标区域发送滚轮，至少一轴非零 |
| `stop` | 无 | 中断输入并撤销控制，后续输入需重新物理启用 |

坐标是竖屏归一化坐标，左上角为原点。滚轮单位不是像素或行数，方向/位移依手机设置及目标，应从小幅操作开始观察。每次调用结束都释放所按输入；多次拖动是分别落笔，不会跨调用持续按住。多点路径将移动时间均分至各段；参数描述固件节奏，USB等待和调度会增加耗时。无保持的两点拖动保留旧直线拖动时序。

系统入口、编辑和光标导航用 `press_key`；静止指针手势用 `tap`；移动/绘图用 `drag`；滚轮调整用 `scroll`。激活焦点或切换状态前先观察。工具不返回组件树、焦点ID、截图或 App 成功结果。

## 按键名称

导航（仅这些允许 repeat>1）：`tab`、`shift_tab`、`up`、`down`、`left`、`right`、`select_left`、`select_right`、`word_left`、`word_right`。

其他键盘键（repeat=1）：`enter`、`escape`、`space`、`backspace`、`delete`、`command_space`、`select_all`、`copy`、`paste`、`cut`、`undo`、`redo`。编辑使用标准 Command/Shift/Alt 组合；`delete` 是向前删除。Space 可能输入空格或激活已有焦点。

系统/媒体键（repeat=1）：`home`、`search`、`toggle_keyboard`、`globe`、`volume_up`、`volume_down`、`mute`、`play_pause`、`next_track`、`previous_track`、`brightness_up`、`brightness_down`。

Home/Search 使用 Consumer 报告，`command_space` 仍是键盘快捷键。Globe 依文本上下文，媒体依当前播放器，音量/亮度是相对调整；切换类动作需观察。不提供通用返回或解锁。系统映射依据 Apple 2026-09-21 配件指南表14-3，见[调研](hid-capabilities.zh-CN.md)。鼠标/键盘ID1/2、Consumer ID3与 `0.5.0-mcp` 相同。

## 参数示例

这些是工具参数，不是已执行操作或完整 MCP 请求。

| 工具 | 示例 |
| --- | --- |
| `tap` | `{"x":0.5,"y":0.6}` |
| `tap`（双击） | `{"x":0.5,"y":0.6,"count":2}` |
| `tap`（长按） | `{"x":0.5,"y":0.6,"holdMs":700}` |
| `drag`（滑动） | `{"points":[{"x":0.5,"y":0.8},{"x":0.5,"y":0.3}],"durationMs":400}` |
| `drag`（绘图） | `{"points":[{"x":0.2,"y":0.3},{"x":0.5,"y":0.6},{"x":0.8,"y":0.3}],"durationMs":800}` |
| `press_key` | `{"key":"select_all"}` 或 `{"key":"home"}` |
| `scroll` | `{"x":0.5,"y":0.6,"vertical":-2}` |

未知字段、重复键、非法枚举/小数/范围、重复激活/媒体键、双击加长按、单击时传 intervalMs、滚轮全零及路径时长不足均在派发前拒绝。完整请求仍限4096字节。

## 兼容调用

以下名称固件继续接受，但不出现在 `tools/list`。它们保留旧参数契约；刷新目录后的 Agent 应用七个公开工具。客户端可能在本地拒绝未列出的工具，因此固件兼容不代表每个客户端都能继续调用。

| 旧名称 | 旧参数 | 新等价调用 |
| --- | --- | --- |
| `keyboard` | `key`，可选 `repeat`、`pressMs`、`gapMs` | `press_key`，字段相同 |
| `system_key` | `action` | `press_key`，key=action |
| `long_press` | `x`,`y`,`durationMs` | `tap`，holdMs=durationMs |
| `double_tap` | `x`,`y`，可选 `intervalMs` | `tap`，count=2 |
| `drag_hold` | `x`,`y`,`toX`,`toY`,`durationMs`，可选 holdMs（默认500） | `drag` 两点路径；旧调用省略保持时需显式传 holdMs=500 |
| `draw_path` | `points`,`durationMs`，可选 `holdMs` | `drag`，字段相同；无保持的两点路径改用旧直线拖动时序 |
| `key` | `keycode`：4…231，`modifiers`：0…255 | 专家原始HID兼容入口，优先命名 `press_key` |

`drag` 还接受旧 `x/y/toX/toY/durationMs` 形式，时序不变；拒绝与 points 混用或在旧形式中新增 holdMs，新接口用 points。默认 `tap`、原始 `key`、`type_ascii`、状态和停止保持旧行为。本次合并不改变描述符、执行器、授权、停止、供电或依赖。

## 结果与停止

扩展动作继续返回文字与结构化 HID 进度：

```json
{"sequenceCompleted":true,"completedUnits":2,"totalUnits":2,"unit":"key_presses","phoneEffect":"unverified","partialEffectsPossible":false}
```

单位是完整按下释放、滚轮报告、双击中的点击、路径段或保持手势。旧单击/两点拖动/原始按键/文本仍返回文字结果。计数是保守进度：未完成单位可能已影响手机，终点释放失败也可能发生在全部路径段完成后。完成不证明 App/任务成功，应再次观察。

全部动作复用单 worker、物理授权、会话归属、输入ID账本、取消与8秒 deadline。停止/取消/失败请求释放；拔线后释放尽力处理。重连不恢复授权、不重放。见[MCP会话](mcp.zh-CN.md)、[预定义验收](hid-development.zh-CN.md)、[验证](validation.zh-CN.md)。源码：[解析](../main/hid_input.c)、[schema](../main/mcp_tools.h)、[执行器](../main/actions.c)、[USB](../main/usb.c)。
