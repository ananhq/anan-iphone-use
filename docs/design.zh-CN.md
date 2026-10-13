# Anan iPhone Use：背景与设计

[English](design.md) · 简体中文

## 为什么使用 StopWatch

Anan iPhone Use（仓库名 `anan-iphone-use`）主打设备端执行的 iPhone 自动化。客户端负责屏幕观察、模型推理、任务规划和用户授权；ESP32-S3 固件将决策转换为 USB 鼠标和键盘输入，不运行模型、不提供手机截图。Anan 是首个客户端，其他 Agent 可按公开协议接入，但需自己的观察能力和明确网络路径。当前仅适配 M5Stack StopWatch，其他硬件按[迁移指南](porting.zh-CN.md)扩展。StopWatch 具备 ESP32-S3 原生 USB、电源管理及两个停止按键，MCP 开发分支现实现中英触屏 UI、实体停止/导航备用，触摸硬件验收仍待完成。新增通路及验收边界见[设备 MCP](mcp.zh-CN.md)。这不是 USB 触摸屏实现：手机收到的是绝对坐标 HID **鼠标**，需要 AssistiveTouch。

```text
手机屏幕 → 手机采屏/客户端 → 模型
模型 / 规划 → 已授权客户端 → USB NCM（HTTP/HTTPS MCP）→ StopWatch → HID → 手机
```

| 归属 | 当前职责 |
| --- | --- |
| 固件 | USB 描述、固定局域网、命令解析、HID、心跳、连接代际所有权和停止 |
| 手机客户端 | 系统屏幕共享/观察、模型推理、任务规划、用户授权、坐标归一化与效果确认 |
| 模型 | 理解提供的截图、选择动作；必须观察真实结果 |

`DONE` 不证明点击命中或任务完成。固件不知道当前 App、控件语义、模型思考状态或完整任务进度；板子不持有云服务或模型凭据。

## 源码组织

[main 模块](../main/README.zh-CN.md)：`board.c` 管供电和按钮，`usb.c` 管 USB/NCM/HID，`main.c` 初始化板级/USB/动作/MCP/UI，`actions.c` 管单 HID worker，`mcp.c` 管 HTTP/HTTPS MCP；旧 TCP 服务/解析器已删除。`build.sh` 调固定 IDF 及 `package.py`；只有显式运行 `flash.sh` 才写设备。依赖从 registry 获取并锁定，不提交第三方源码。产物保留历史 `anan-stopwatch-*` 名称便于追溯。

## 参考与差异

- [actell 调研版本](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904)：保留的 `firmware-device` 方案使用手机采屏 + NCM/HID。后来的 CoreDevice 路线涉及不同设备协议/配对职责，本仓库不包含。用户确认参考了 actell 源码；本固件 USB 网络实现有较多相似结构，逐文件衍生关系和来源处理尚待核实，详见[来源记录](../THIRD_PARTY.zh-CN.md)。
- [esp32-iphone-agent 调研版本](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18)：电脑 AirPlay/Python Agent 通过 Wi-Fi 控制 Arduino 固件。本方案采屏/Agent 在手机，命令走 USB 以太网，固件采用 ESP-IDF。
- [M5Stack 官方文档](https://docs.m5stack.com/zh_CN/core/StopWatch)提供板型和下载模式信息。它是触控 AMOLED 设备；本次没有迁移图形或触摸实现。

Anan 的后台采屏、流式调度和灵动岛留在 Anan。拆分固件不会赋予任意手机客户端持续后台执行能力，也不会自动打通其他 Agent 的网络。本次不改变 prompt、历史或缓存前缀。
