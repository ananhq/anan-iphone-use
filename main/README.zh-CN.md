# 固件模块

[English](README.md) · 简体中文

面向 M5Stack StopWatch 的单一 ESP-IDF 组件。`main.c` 仅负责初始化；
`bridge.h` 保留现有板级/输入公共接口。子目录用于组织代码，不增加组件、任务或运行时抽象。

| 目录 | 模块 | 职责 |
| --- | --- | --- |
| [app/](app/) | `device.c/h, reset_trace.c/h` | 设备状态、物理控制授权、停止，以及启动时锁存的重启诊断。 |
| [hardware/](hardware/) | `board.c, battery.c/h, display.c/h, panel_init.h, touch.c/h, imu.c/h, tilt.c/h` | StopWatch 供电与引脚、只读电量估算、屏幕/触摸驱动及 BMI270 体感采样。 |
| [input/](input/) | `actions.c/h, hid_input.c/h, input.c, input_backends.h, usb.c, ble_input.c, ble_reports.c/h` | 单一 HID worker、有界动作解析、显式 USB/BLE 选择与报告发送。 |
| [network/](network/) | `mcp.c/h, mcp_tools.h, identity.c/h, wireless.c/h, network_store.c/h, network_authority.c/h, discovery.c/h, wifi_qr.c/h` | HTTP/HTTPS MCP、会话/配对、身份、Wi-Fi/热点配置、网络存储、Host/Origin 校验、mDNS 与二维码载荷。 |
| [ui/](ui/) | `ui.c/h, ui_layout.c, ui_font.h, ui_preferences.c/h, eyes.c/h, avatar.cpp/h, avatar_native.cpp/hpp, avatar_activity.cpp/hpp, avatar_timing.c/h` | 双语页面布局/偏好、输入反馈事件、原生表情绘制、MCP 操作道具与耗时诊断。 |
| [vendor/](vendor/) | `stopwatch_avatar/, bmi270/, qrcodegen.c/h` | 固定版本的第三方表情引擎、传感器 API 和二维码编码器，保留来源及逐文件声明。 |
| [web/](web/) | `setup.html` | 嵌入式双语配对、Wi-Fi 配置及 MCP 导出网页，不包含 Agent/模型代码。 |

客户端负责观察、推理、规划和用户授权；固件不运行模型、不提供手机截图。
USB 与 BLE 显式选择，不自动回退。命令、请求和控制授权不持久化；停止、断线或
重启不恢复控制，也不重放动作。实体停止不依赖显示、I2C 或网页。

控制画面使用 **[Trentct/m5stack-stopwatch-avatar](https://github.com/Trentct/m5stack-stopwatch-avatar)**
的 AGPL-3.0-or-later 引擎，原生移植与 MCP 动作反馈见
[来源与修改](vendor/stopwatch_avatar/ORIGIN.md)、[跟手动画](../docs/history/avatar-responsive.zh-CN.md)。
`eyes.c` 仍提供输入事件与旧备用绘制路径，本次目录整理不删除或改变它们。

默认 SDK 配置未启用 PSRAM；紧凑头像缓存与重启诊断见
[网络内存记录](../docs/history/network-startup.zh-CN.md)。用户已报告配网故障解决，
持续运行和重连耗时仍需分别验证；编译或离线检查不代表真机验收。

从根目录使用 `./build.sh` 构建；[仓库组织](../docs/repository.zh-CN.md)说明目录和命令，
[工具索引](../tools/README.zh-CN.md)列出现有离线检查。
[MCP](../docs/mcp.zh-CN.md)、[无线配置](../docs/wireless.zh-CN.md)描述当前行为；
历史候选与逐阶段证据统一见[验证记录](../docs/validation.zh-CN.md)。
[来源声明](../THIRD_PARTY.zh-CN.md)与[源码提供](../SOURCE.zh-CN.md)保留适用条款。
