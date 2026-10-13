# 固件模块

[English](README.md) · 简体中文

已实现的 StopWatch 专用 ESP-IDF C 模块。`board.c` 配置 I²C SDA 47 / SCL 48、M5PM1 地址 0x6e / 寄存器 0x07 bit 5，以及 GPIO 1/2 低电平按键；保持 MCU LDO，不启用外部 5V 升压。原生 USB 使用 GPIO 19/20。

`usb.c` 实现全速 NCM+HID 组合设备、lwIP IPv4 网卡、有超时的同步发包、绝对鼠标与键盘报告。`bridge.h` 暴露板级初始化/停止检测、USB 初始化/在线检测、鼠标/键盘/释放接口；只有动作 worker 提交 HID。`main.c` 管 TCP 28766、解析、易失连接代际与心跳、单槽队列和 worker。

[PROTOCOL.md](../PROTOCOL.md) 是线协议/动作/终态规则权威。命令、授权和连接状态均不持久化。依赖见 [idf_component.yml](idf_component.yml)、[锁文件](../dependencies.lock)及 IDF 内置 GPIO/I²C/netif/timer/lwIP。仓库根目录运行 `./build.sh`；详见[验证](../docs/validation.zh-CN.md)与[来源](../THIRD_PARTY.zh-CN.md)。

当前没有显示、触摸屏、Wi-Fi/BLE、PSRAM 分配、截图服务或 MCP。拔线后释放按键是尽力操作；源码/构建证据与真机验收分开。
