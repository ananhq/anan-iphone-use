# 固件模块

[English](README.md) · 简体中文

Anan iPhone Use（`anan-iphone-use`）：面向 M5Stack StopWatch 的 ESP-IDF C 固件，在设备端执行 iPhone 输入；客户端负责观察、推理、规划和用户授权，固件不运行模型、不提供 iPhone 截图。编译/静态结果不代表手机、显示、证书信任、延迟或互通验收。

| 模块 | 职责 |
| --- | --- |
| `board.c`、`bridge.h` | 保留 SDA47/SCL48、M5PM1 0x6e 寄存器0x07 bit5 MCU供电保持、GPIO1/2低电平停止；导出现有 I2C 总线供显示使用 |
| `usb.c` | 原 USB 全速 NCM/绝对坐标鼠标/键盘实现，与检查点字节相同 |
| `main.c` | 仅初始化板级/USB/动作/MCP/UI；不再包含 ANAN1 TCP 监听/解析/心跳 |
| `actions.c/h` | 共用单 worker/槽，保留原 US ASCII 映射与 HID 报告/暂停顺序；调用方提供存活检查和完成回调 |
| `device.c/h` | 触屏/实体备用操作、原始按键停止、MCP 授权持续至停止/拔线/重启；独立触摸/低优先级显示任务 |
| `mcp.c/h`、`mcp_tools.h` | 有界 HTTP/HTTPS MCP2025-11-25、六个工具、会话归属/ID记账、异步响应；仅 USB 配对/证书/配置及只读启动诊断 |
| `identity.c/h` | 硬件熵初始化 DRBG，生成 CA/证书/token，独立 NVS 身份分区；显式维护/重置 |
| `display.c/h`、`panel_init.h` | CO5300 QSPI 条带渲染，M5IOE1 唤醒/重试/处理间隔及显示供电/复位回读，自写 ASCII 字形；保留 MIT 厂商命令表声明 |
| `touch.c/h` | CST820B 0x15 原生有界轮询、GPIO13、扩展芯片 PYG4 复位；触摸错误取消手势 |
| `ui.c/h`、`ui_layout.c`、`ui_font.h` | 共用绘制/命中布局、中英标签、SIL OFL 中文字形子集、当前已显示画面快照 |
| `ui_preferences.c/h` | 独立 ui NVS 命名空间记录语言，不持久化控制授权 |
| `web/setup.html` | 双语手机配对、证书说明、HTTP 优先 JSON 导出、可选 HTTPS，不含 Agent/模型代码 |

[MCP 文档](../docs/mcp.zh-CN.md)描述当前唯一控制协议与配置；
[PROTOCOL.md](../PROTOCOL.md) 保持为未改写的历史 ANAN1 契约。
旧服务和控制来源切换 UI 已删除，默认进入 MCP，但不自动授权控制。命令、控制授权、请求、连接不持久化；生成的 TLS/配对身份及独立语言偏好持久化。
保留 USB 描述符及原 MCU 供电保持/5V boost 行为，显示供电单独初始化。
不含 Wi-Fi/BLE、PSRAM分配、截图服务、模型 provider 或复制的 [Anan](https://anan.sh) 客户端。

仓库根目录使用固定 ESP-IDF5.5.5 和 `./build.sh` 构建；准确依赖见
[manifest](idf_component.yml)、[锁文件](../dependencies.lock)、[声明](../THIRD_PARTY.zh-CN.md)。
生成文件、日志和凭据保持忽略。

[触屏 UI](../docs/touch-ui.zh-CN.md)记录硬件/交互及预定义检查。默认中文，设置可选
English，记忆语言；手机页面也可切换。实体键备用：黄键选、蓝键确认；选中重置后
按住蓝键两秒。触摸或实体键停止不复用同次接触重新授权。`tools/`仅生成字体和静态
文档预览，不是设备/模拟器/E2E 或固件运行时依赖。
