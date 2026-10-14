# HID 扩展验收范围

[English](hid-development.md) · 简体中文

2026-10-14 已同意：一次实现多项固件能力，通过清晰的 MCP 工具提供。新增功能不依赖手机自定义按键映射、文本替换或开启全键盘控制；保留原有 USB 网络/AssistiveTouch/配对/控制设置。Anan 源码不在修改范围内。

候选 `0.5.0-mcp`：保留原六个工具，新增 `keyboard`、`scroll`、`long_press`、`double_tap`、`drag_hold`、`draw_path`、`system_key`。追加 Consumer report ID 3，保留鼠标 ID 1、键盘 ID 2、USB 接口、电源和旧动作时序。不升级依赖或改变许可，不提供跨调用永久按住，不自动重试/重放。

实施前定义离线检查：schema/解析一致与非法参数；命名修饰键、禁止重复激活；滚轮符号；双击间隔；拖前保持；连续路径插值/端点；按住期间中断；完成单位进度；Consumer 零报告释放与旧报告兼容。这些是模拟报告/时间的主机检查，不是真机响应或 USB 枚举证明。

另行授权的真机范围：NCM/鼠标/键盘/Consumer 共同枚举；旧 tap/drag/key/ASCII 回归；列表中的小幅纵向/横向滚轮；稳定目标的长按/双击/保持后拖动；合成绘图；未开启全键盘控制时普通键盘/编辑；逐项 Home/Search/键盘显示/媒体/音量。检查按住期间 stop/cancel/拔线、释放/不重放，以及 Anan 刷新工具目录。使用合成数据，记录未知/部分结果、时序/设置/上下文；不能为了通过验收临时改手机绑定。若已开启全键盘控制，可另测焦点导航。

实现/编译结果与真机验收分开记录。固件只报告输入发送、完成单位与可能部分效果，不宣称焦点/App 成功。Consumer 语义依据锁定 Apple 2026-09-21 指南及 TinyUSB API；[调研](hid-capabilities.zh-CN.md)保留证据与限制。

## 工具合并范围（2026-10-14）

用户已同意把工具发现从十三项收敛为七项：`device_status`、`tap`、`drag`、`press_key`、`type_ascii`、`scroll`、`stop`。候选 `0.5.1-mcp` 保留旧调用兼容入口，但不放入 `tools/list`；需要刷新工具目录。`tap` 增加可选 `count`（1/2）、`holdMs`（0或100…2000）、`intervalMs`（仅双击），拒绝双击与长按组合。`drag` 使用2…32个点、时长及可选拖前保持；无保持的两点路径保留旧直线拖动时序。旧 `x/y/toX/toY` 参数继续可调用，但不展示。`press_key` 合并命名键盘与 Consumer 按键，原始 `key` 保留为隐藏兼容入口。USB描述符、执行器、供电、授权、停止、依赖和客户端代码不变。

实施前定义：运行已有扩展验收场景；增加离线解析/执行场景，检查默认点击时序、长按/双击派发及非法组合；两点路径时序等价、保持/多点路径和新旧拖动参数混用拒绝；命名/Consumer 按键等价、重复限制与完整按键映射；工具列表恰好七项、简短说明及枚举与解析一致。本轮不授权设备操作。另行授权的真机验收应检查 Anan 刷新工具、旧调用、新合并参数与中断；编译/离线结果不证明这些行为。

## 复现离线检查

加载 ESP-IDF v5.5.5 后运行 `bash tools/check_hid.sh`。实施前场景及最小时间/FreeRTOS/报告替身位于 `tools/hid_acceptance/`，编译实际动作执行/解析和锁定 cJSON 源码，不调用真实 USB 或网络；不代表线程、SDK驱动、客户端或真机验收。

隔离候选使用 `idf.py -B build/hid-merged-candidate build`、`python package.py build/hid-merged-candidate` 构建/打包，镜像与manifest从同一目录取。其他检出路径的旧缓存没有删除，也没有当作本候选产物。
