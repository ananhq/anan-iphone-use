# 路线图与交接

[English](roadmap.md) · 简体中文

Anan iPhone Use（`anan-iphone-use`）在 M5Stack StopWatch 上执行 iPhone 动作；客户端负责观察、推理、规划和授权。仓库地址为 [ananhq/anan-iphone-use](https://github.com/ananhq/anan-iphone-use)，保持私有，公开发布仍待完成；不因命名改变 ANAN1、USB 标识、工具或历史产物。见[GPL许可与来源处理](license-plan.zh-CN.md)。

| 阶段 | 状态/出口 |
| --- | --- |
| 迁移 | 本地源码/文档/编译检查点 `4c6015c`，无公开 remote 或 Release |
| 基线验收 | 用户反馈重新烧录成功并授权检查点；手机输入/停止及来源核实仍待完成 |
| MCP 与必要 UI | 已实现至候选 `0.4.0-mcp`，编译/静态预览通过；用户反馈屏幕亮起及 MCP 满意，具体验收中的触摸/记忆/互通/停止仍待完成 |
| 明文 HTTP MCP | 已实现 HTTP80 MCP 和同页配对/配置导出；用户反馈另一 [Anan](https://anan.sh) session 已完成 HTTP 支持，用户反馈上一候选 HTTP 新用户流程已验证良好，新的仅 MCP 镜像仍待真机验证 |
| 仅 MCP | 按用户要求移除 ANAN1 28766端口/解析/心跳/来源切换；保留 USB NCM/HID，默认 MCP 但不授权输入 |
| HID 扩展 | 候选 `0.5.1-mcp`：七个展示工具，保留兼容调用，保留原鼠标/键盘 ID/时序并追加 Consumer ID 3；不要求新增手机绑定/全键盘控制；真机验收待完成 |
| 无线 | `0.6.0-wireless` 加扫描候选 `0.6.1-wifi-scan`：Wi-Fi HTTP MCP、USB/BLE 显式输入、首次 USB 扫描/选择配网；BLE/iPhone、实际射频扫描与共存验收待完成 |
| 更多显示 | 触屏/双语设置已获授权，动画/模型思考仍不在范围 |

本地检查点不等于完整稳定基线验收，保留源码及历史证据便于恢复。Anan 管采屏、Agent、
原生工具、授权、后台和模型；固件管输入及设备配置。用户明确禁止此阶段修改 Anan，
暂缓进一步观察修改；简单按钮/菜单 UI 随 MCP 配置实现。
见[MCP 决策](mcp-development.zh-CN.md)与[实现/配置](mcp.zh-CN.md)。

[PROTOCOL.md](../PROTOCOL.md) 保留历史 ANAN1 契约，运行服务按用户要求退役。
MCP 是当前唯一控制协议，使用 HID worker、保留
原鼠标/键盘报告与旧时序（`0.5.0-mcp` 追加 Consumer ID 3）、明确物理授权/会话归属、不隐式重试或重连输入。身份存储与控制授权
分开；USB 可达性和 Anan 原生观察门槛仍有限制，MCP 不提供截图或远程客户端网络桥。

允许编译/静态检查；烧录、设备操作、provider、模拟器/E2E 要对当前范围另获授权。
分别记录实际结果，编译不代表硬件通过，不在实现后补单元测试。不默认委派或在 Anan
下恢复第二份固件。重要架构/协议/依赖/许可变更实现前讨论，公开前解决原来源缺口和
链接组件声明；改变项目许可证需确认。

贡献先约定范围，保留来源及双语文档。本地 commit 与公开发布分开；未获发布授权不
push、创建公开仓库或 Release。见[基线验收](baseline-acceptance.zh-CN.md)和
[开源发布准备](release-preparation.zh-CN.md)。

用户追加触屏为主及设备内中英切换/记忆的要求，现单独实现，见[触屏决策](touch-ui.zh-CN.md)；
该阶段不改 Anan/ANAN1，不扩大设备执行授权；后续用户明确要求的仅 MCP 阶段退役 ANAN1 服务。

无线配网候选 `0.7.0-wireless-setup` 实现已确认的 USB/Wi-Fi 固定域名、五个网络记忆及临时 SoftAP 配网。[当前配置](wireless.zh-CN.md)、[预定义范围](wireless-setup-development.zh-CN.md)及[验证](validation.zh-CN.md)取代早期 USB-first/数字 IP 流程，不宣称完成新的设备实测或稳定验收。

候选 `0.7.2-wifi-qr` 跟进用户 `.local` 打不开的反馈，修正已从源码复现的锁定 lwIP 定时器在仅 IPv4 网卡上强制等待 IPv6 探测的问题；增加屏幕 Wi-Fi 二维码，保留随机 WPA2 密码和物理配对。定时器回归与二维码场景均先于修改定义。SDK 编译/打包、既有离线场景、独立二维码及完整圆屏静态预览解码通过，见[记录](../artifacts/wireless/qr-build-check.json)。Agent 未烧录、探测射频/串口、使用手机相机/加入网络、连接真实客户端/模型或执行 E2E，IPv4 重名处理及新的手机 `.local` 成功仍待实机验证。

候选 `0.7.3-discovery` 是已确认的纯发现元数据阶段：双语任务词及柔性整组概览建议，不新增工具、改变 schema 或执行。预定义范围和限制见[工具发现](tool-discovery.zh-CN.md)，不从编译/静态证据推断模型采用或手机操作成功。
