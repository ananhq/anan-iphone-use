# 配置、烧录与恢复

[English](setup.md) · 简体中文

## 电脑上构建一次

使用 StopWatch 原生 Type-C 口和数据线。递归安装官方 ESP-IDF **v5.5.5**，执行 `install.sh esp32s3` 后 source `export.sh`，确保 CMake ≥3.22 / Ninja；其他 IDF/Arduino 环境不属于该构建。首次按锁文件获取 esp_tinyusb 2.2.0 / TinyUSB 0.21.0~2。

```sh
./build.sh
shasum -a 256 build/anan-stopwatch-flash.bin
```

与 `build/anan-stopwatch-manifest.json` 校对哈希；其中记录源码、依赖锁哈希、镜像大小与验证标记。镜像合并引导程序、分区和应用。目标 ESP32-S3、**16MB**、合并写入地址 **0x0**，不要把合并镜像写到应用偏移。尚无公开 Release 二进制，请本地构建；历史与迁移镜像哈希在[验证记录](validation.zh-CN.md)中分开记录。

## 确认目标后才烧录

按[官方下载模式说明](https://docs.m5stack.com/zh_CN/core/StopWatch)：接电脑，长按电源约 2 秒至绿色 LED 亮后松开。通过插拔识别实际串口，不要照抄占位符。

```sh
./flash.sh /dev/cu.usbmodemXXXX
# 已进入下载模式，但正常复位无法连接时：
python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX \
  --baud 460800 --before no_reset write_flash --flash_size 16MB \
  0x0 build/anan-stopwatch-flash.bin
```

确认写入与哈希校验成功，再短按电源复位。MCP 开发版会初始化简单状态/菜单屏幕，黑屏不再是其预期 UI。烧录替换出厂固件，恢复时使用官方 M5Burner 中对应 StopWatch 的出厂包和说明。脚本不修改 eFuse、不启用 Secure Boot。本次迁移不烧录设备。

## 手机配置

| 项目 | 值/操作 |
| --- | --- |
| 接线 | StopWatch Type-C → 解锁、竖屏的 USB-C iPhone |
| IPv4 | USB 以太网手动 192.168.7.2，掩码 255.255.255.0；路由器空，不添加 DNS |
| 指针 | 设置 → 辅助功能 → 触控 → AssistiveTouch 开启 |
| 键盘 | 外接硬件 US 布局；type_ascii 只支持可打印 ASCII、TAB/LF |
| 客户端 | 允许局域网权限；导入/连接设备 MCP 配置，开启客户端屏幕共享以观察 |
| Agent | 配置支持图片的模型和客户端任务许可；在 StopWatch 点“开始控制”授权硬件输入 |

Anan 采屏要求 iOS 27+。USB 子网不是互联网出口，模型仍使用手机原有 Wi-Fi/蜂窝网络。HID/以太网本身不需要开发者模式；当前 Anan 开发签名安装需要，TestFlight/App Store 分发未在本次验收。

先只观察，再在空白测试笔记做单个小输入，每次动作后观察。客户端任务许可与板上 MCP 授权分开。任一正面按键、MCP stop 或拔线均可中断输入；显式重连并检查局部效果，禁止自动重放失败命令。

## 排错

| 现象 | 下一步 |
| --- | --- |
| 没有以太网 | 数据线、退出下载模式/复位、正确 16MB镜像 |
| MCP连接失败/超时 | 手动IP/掩码、局域网权限、HTTP端点及导入的Bearer；不再使用28766端口 |
| 预览异常 | 解锁/竖屏、全屏共享同意；受保护内容可能不可见 |
| 指针/点击不对应 | AssistiveTouch 和浮动菜单；停止并检查坐标 |
| 字符不对 | US 硬件布局；不支持 Unicode/IME |
| 后台停滞 | 检查手机客户端采屏/执行状态；固件不为 iOS 保活 |
| 部分文字/结果未知 | 人工观察、重连/重新授权；不要重放 |

使用合成屏幕/空白笔记，日志不收私人截图或设备标识。后续真机执行需独立授权范围，记录固件哈希、客户端版本、系统和观察结果。

设备端 MCP 的模式按钮、HTTP 优先配置导出和可选 HTTPS 证书见[配置步骤](mcp.zh-CN.md)。
