# 客户端接入

[English](client.md) · 简体中文

候选 **0.7.2-wifi-qr** 增加扫码临时热点配网、修正仅 IPv4 时的 mDNS 探测、五个 Wi-Fi 网络记忆及 USB/Wi-Fi 共用的 `http://stopwatch.local/mcp` HTTP 端点。BLE 输入仍为实验性。保留数字 USB 配置入口和可选 USB HTTPS，见[当前无线流程与限制](wireless.zh-CN.md)；下文早期阶段说明适用于原范围。

Anan iPhone Use（仓库名 `anan-iphone-use`）是设备端执行器。[Anan](https://anan.sh) 是首个客户端，其他 Agent 可按公开 [MCP 端点](mcp.zh-CN.md)接入。当前仅适配 M5Stack StopWatch；客户端负责观察、推理、规划和用户授权，板子负责 USB/实验性 BLE 鼠标/键盘执行。

## 客户端 iPhone Use 能力描述

同时开放观察和硬件输入的客户端工具组推荐 description（仅文档，不表示 Anan 当前实现已采用）：

> 观察并操作当前 iPhone。通过系统屏幕共享获取画面，通过已连接的硬件执行点击、拖动、按键和 ASCII 输入。需要用户开启本次任务控制；动作后应重新观察，确认实际效果。

`observe` / Anan 的 `phone.observe` 由手机客户端通过系统屏幕共享提供，不属于固件或其 MCP 工具；`tap`、`drag`、`press_key`、`type_ascii`、`scroll` 由硬件执行。HID `DONE` 或 MCP 工具结果仅确认输入序列，不代表目标操作成功；每次动作后应获取新截图确认实际效果。

手机内客户端与电脑端 Agent 的网络可达条件不同。直连手机可访问 USB 子网，电脑 Agent 可使用已配置 Wi-Fi 局域网，云端仍需明确网络路径，二者自行提供截图，不能仅凭支持 MCP 就直接访问手机 USB 子网。第三方互通仍未验证。实际 Anan 工具描述代码归另一 Anan 仓库/session，本次不修改。

本次只读检查的 Anan 源码快照中，`apps/ios/Anan/Tools/Phone/PhoneTools.swift` 将原生工具组过滤为 `phone.observe`，并拒绝原生输入工具派发。因此，上述描述是推荐的组合能力，不代表 Anan 当前已开放全部原生手机工具。硬件输入使用固件 MCP 工具；旧 Anan 直连 TCP 输入已退役。已检查的观察就绪条件不要求输入连接。

## MCP 接入

1. 按[配对/导入](mcp.zh-CN.md)使用共用 `http://stopwatch.local/mcp` HTTP 地址（保留数字 USB/LAN 回退入口）或可选 USB HTTPS8443，带板上 Bearer 请求头。使用 Anan MCP 设置，不再使用旧“直连 StopWatch” TCP 连接入口。
2. 初始化 MCP，保留会话/版本请求头，发送 initialized 通知并发现七个工具；状态/发现不授权手机输入。
3. StopWatch 点“开始控制”，首个输入会话拥有该授权。`tap`、`drag`、`press_key`、`type_ascii`、`scroll` 执行已选择的 USB/BLE HID 后端，`device_status` 和 `stop` 查看/撤销控制，见[工具 schema 与会话规则](mcp.zh-CN.md)。
4. 使用竖屏归一化坐标0…1、最多120字节的 US 布局 ASCII、发现的命名按键枚举（原始 usage 仅供隐藏兼容入口），不发送历史 ANAN1 行格式。
5. 优先使用[扩展 HID 工具](hid-tools.zh-CN.md)的命名键盘/系统按键与有界手势；升级后刷新工具发现。不要求新增手机绑定或开启全键盘控制；可选焦点导航依已有系统设置。
6. 动作前后观察屏幕。完成仅确认输入派发，不代表 App 成功；停止/取消后检查未知局部结果，不重放。选定输入掉线、正在使用的 Wi-Fi 丢失或重启撤销权限，重连不恢复输入授权。

从 `0.4.0-mcp` 起，28766端口、ANAN1 READY/PING/STOP 和直连/MCP来源切换均不再存在。[PROTOCOL.md](../PROTOCOL.md)仅供历史版本使用，不是当前接入选项。USB以太网与HID仍使用同一数据线；MCP不提供截图，也不自动打通电脑/云端至USB子网的网络路径。

## 任务词与整组概览

候选 `0.7.3-discovery` 保留七个工具 schema，增加简短中英文用途词。打开应用是主屏幕/Spotlight、按需 ASCII 输入、点击观察到的图标/结果组成的 UI 流程，不是 bundle ID 启动器。多步控制时，服务建议在概览尚不可见时，用空 query、准确客户端 group、limit=10 浏览整组，再读取选定工具参数。简介不加载完整 schema 或授予控制。Anan 自定义服务文字可能覆盖此建议，刷入后应刷新发现。见[细节及预定义检查](history/tool-discovery.zh-CN.md)，本固件阶段不修改 Anan 客户端。
