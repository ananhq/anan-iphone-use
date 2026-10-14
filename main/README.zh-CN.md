# 固件模块

[English](README.md) · 简体中文

Anan iPhone Use（`anan-iphone-use`）：面向 M5Stack StopWatch 的 ESP-IDF C 固件，在设备端执行 iPhone 输入；客户端负责观察、推理、规划和用户授权，固件不运行模型、不提供 iPhone 截图。编译/静态结果不代表手机、显示、证书信任、延迟或互通验收。

| 模块 | 职责 |
| --- | --- |
| `board.c`、`bridge.h` | 保留 SDA47/SCL48、M5PM1 0x6e 寄存器0x07 bit5 MCU供电保持、GPIO1/2低电平停止；导出现有 I2C 总线供显示使用 |
| `input.c`、`input_backends.h` | 显式 USB/BLE 选择与串行报告派发，不自动回退 |
| `ble_input.c`、`ble_reports.c/h` | 实验性加密 NimBLE HID、订阅/完成守卫、绝对定位与相对按键分开发送 |
| `wireless.c/h` | 五个 Wi-Fi 网络、轮换连接、临时 WPA2 配网热点、IPv4 状态及有界异步扫描 |
| `network_store.c/h`、`network_authority.c/h`、`discovery.c/h` | 有界 v1/v2 网络存储、HTTP Host/Origin 校验、USB/Wi-Fi 上的锁定 lwIP mDNS |
| `wifi_qr.c/h`、`vendor/qrcodegen.c/h` | 第一方有界 WPA2 Wi-Fi 载荷与适配、锁定且原样保留的 Nayuki MIT 编码器，原生分块绘制四模块静区 |
| `usb.c` | USB NCM 与保留的绝对鼠标/键盘 ID；滚轮发送及追加 Consumer ID 3 |
| `main.c` | 初始化板级/USB/输入/动作/MCP/UI/Wi-Fi/BLE；不再包含 ANAN1 TCP 监听/解析/心跳 |
| `actions.c/h` | 共用单 worker/槽，保留原 US ASCII 映射与 HID 报告/暂停顺序；调用方提供存活检查和完成回调 |
| `hid_input.c/h` | 合并手势/按键与旧调用兼容的严格有界解析、命名键盘/Consumer 映射与进度单位 |
| `device.c/h` | 触屏/实体备用操作、原始按键停止、MCP 授权持续至停止/拔线/重启；独立触摸/低优先级显示任务 |
| `mcp.c/h`、`mcp_tools.h` | 有界 HTTP/HTTPS MCP2025-11-25、七个展示工具及旧兼容入口、会话归属/ID记账、异步响应；USB/LAN 配对与配置及只读诊断；Wi-Fi 管理仅限物理配对后的 USB/热点；证书/HTTPS 仍仅 USB |
| `identity.c/h` | 硬件熵初始化 DRBG，生成 CA/证书/token，独立 NVS 身份分区；显式维护/重置 |
| `display.c/h`、`panel_init.h` | CO5300 QSPI 条带渲染，M5IOE1 唤醒/重试/处理间隔及显示供电/复位回读，自写 ASCII 字形；保留 MIT 厂商命令表声明 |
| `touch.c/h` | CST820B 0x15 原生有界轮询、GPIO13、扩展芯片 PYG4 复位；触摸错误取消手势 |
| `ui.c/h`、`ui_layout.c`、`ui_font.h` | 共用绘制/命中布局、中英标签、SIL OFL 中文字形子集、当前已显示画面快照 |
| `ui_preferences.c/h` | 独立 ui NVS 命名空间记录语言/输入选择，不持久化控制授权 |
| `web/setup.html` | 双语配对、扫描/选择/手填 Wi-Fi、HTTP 优先 JSON 导出及可选 USB HTTPS，不含 Agent/模型代码 |

[MCP 文档](../docs/mcp.zh-CN.md)描述当前唯一控制协议与配置；
[PROTOCOL.md](../PROTOCOL.md) 保持为未改写的历史 ANAN1 契约。
旧服务和控制来源切换 UI 已删除，默认进入 MCP，但不自动授权控制。命令、控制授权和请求不持久化；BLE 绑定与 Wi-Fi 独立保存，重连不恢复输入；生成的 TLS/配对身份及独立语言偏好持久化。
保留原鼠标/键盘 ID 和旧时序，追加 Consumer ID 3；保留原 MCU 供电保持/5V boost 行为，显示供电单独初始化。
Wi-Fi/BLE 为实验性实现，不含 PSRAM分配、截图服务、模型 provider 或复制的 [Anan](https://anan.sh) 客户端。

仓库根目录使用固定 ESP-IDF5.5.5 和 `./build.sh` 构建；准确依赖见
[manifest](idf_component.yml)、[锁文件](../dependencies.lock)、[声明](../THIRD_PARTY.zh-CN.md)。
生成文件、日志和凭据保持忽略。

[触屏 UI](../docs/touch-ui.zh-CN.md)记录硬件/交互及预定义检查。默认中文，设置可选
English，记忆语言；手机页面也可切换。实体键备用：黄键选、蓝键确认；选中重置后
按住蓝键两秒。触摸或实体键停止不复用同次接触重新授权。`tools/`生成字体/静态预览，并运行实施前编写的离线 HID 场景，均不是设备/模拟器/E2E 或固件运行时依赖。见[扩展工具](../docs/hid-tools.zh-CN.md)。

[无线配置与证据](../docs/wireless.zh-CN.md)记录候选 `0.7.0-wireless-setup`，包括默认 NVS 蓝牙绑定恢复、临时热点配网、固定域名及后端切换。Wi-Fi、BLE 绑定、输入选择独立持久化，不恢复控制或动作。

`0.7.0-wireless-setup` 管理任务串行处理 Wi-Fi 写入、扫描及模式切换。临时 WPA2 热点限一台客户端，使用 DHCP 和仅在屏幕显示的随机密码，暂停 BLE 且不恢复控制授权。五网络 v2 存储读取 v1 记录；HTTP Host/Origin 校验绑定实际本地 socket。`discovery.c` 使用锁定 lwIP 的 USB/STA/AP mDNS responder，增加一个 client-data 槽位；USB 仅增加 IGMP，保留 HID 报告和发送器。固定域名冲突明确报告，不静默更改已导入 URL。

候选 `0.7.2-wifi-qr` 增加仅在屏幕显示的热点二维码，保留手动密码、WPA2 与物理配对。根 CMake 编译生成的哈希校验 lwIP mDNS 适配，跳过不可用的 IPv6 探测，保留系统 IPv6 配置、原 SDK 检出和 IPv4 名称冲突探测。独立解码合成图片和 SDK 定时器模拟不能证明手机/运行时验收。

`0.7.3-discovery` 仅修改七个工具描述、initialize 简介/instructions 及候选版本。中英文任务词将输入动作关联到打开应用、导航、输入/编辑、绘图、滚动及媒体任务。完整概览为可选建议，完整参数读取和控制授权仍分开，不改变解析器、执行器、报告、通道或 Anan。见[工具发现说明](../docs/tool-discovery.zh-CN.md)。
