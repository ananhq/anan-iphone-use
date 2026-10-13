# StopWatch 固件

[English](README.md) · 简体中文

面向 **M5Stack StopWatch（ESP32-S3、16MB Flash）** 的 USB 以太网与 HID 输入固件。手机内的 Agent 负责看屏幕，一根 USB-C 数据线向板子发送动作；板子模拟绝对坐标鼠标和键盘。Anan 是首个客户端，固件不运行模型、不获取手机截图。

本仓库从 Anan 拆出，正在整理开源发布所需内容。名称/地址暂定，尚无公开仓库或 Release。
本地检查点记录编译/静态检查及用户反馈的重新烧录成功；手机输入/停止验收和来源核实仍待完成。
当前范围是 StopWatch 的 USB 操控；Wi-Fi、BLE、MCP 和显示/触摸 UI 留到后续阶段。

```text
手机采屏 → 模型 → 手机客户端 → USB 以太网 / ANAN1
                                      ↓
                            StopWatch → USB HID → 手机
```

## 构建与使用

安装 ESP-IDF **v5.5.5**，先 source 其 `export.sh`：

```sh
./build.sh
shasum -a 256 build/anan-stopwatch-flash.bin
```

构建/打包不会访问串口。合并镜像与源码/哈希 manifest 生成在被忽略的 `build/` 下。
指定实际串口烧录和手机配置见[配置与恢复](docs/setup.zh-CN.md)。烧录替换出厂固件，
设备黑屏是当前预期。运行时使用手机原有模型连接，板子不需要 Mac、Wi-Fi 或 BLE。
Anan 当前采屏客户端要求 iOS 27+。

用户已提供候选镜像成功写入/哈希校验反馈，并授权首次本地检查点提交。
手机行为仍需按[基线验收步骤](docs/baseline-acceptance.zh-CN.md)明确范围，
历史输入反馈不能代替本次验证。

## 目录

| 路径 | 职责 |
| --- | --- |
| `main/` | StopWatch 供电/按钮、USB 网卡/HID、命令执行 |
| `build.sh`、`package.py`、`flash.sh` | 固定 IDF 构建、镜像打包、显式设备写入 |
| `sdkconfig.defaults`、`dependencies.lock` | 提交的默认配置与准确依赖解析 |
| `docs/` | 双语设计、配置、接入、验收和路线图 |
| `artifacts/` | 脱敏迁移与基线证据，不存生成镜像 |
| `licenses/`、`THIRD_PARTY.md`、`NOTICE` | 依赖声明与实现来源记录 |

## 文档

- [背景与职责](docs/design.zh-CN.md)、[客户端接入](docs/client.zh-CN.md)。
- [ANAN1 协议](PROTOCOL.md)：唯一权威的线协议/生命周期定义；当前没有 HTTP/MCP。
- [适配指南与 Prompt](docs/porting.zh-CN.md)、[验证记录](docs/validation.zh-CN.md)、[路线图与交接](docs/roadmap.zh-CN.md)。
- [第三方来源](THIRD_PARTY.zh-CN.md)、[LICENSE](LICENSE)、[NOTICE](NOTICE)、[源文件快照](artifacts/migration/source-snapshot.json)。
- [基线验收](docs/baseline-acceptance.zh-CN.md)、[开源发布准备](docs/release-preparation.zh-CN.md)、[贡献说明](CONTRIBUTING.zh-CN.md)、[完整文档导航](docs/README.zh-CN.md)。

首版仅竖屏、单指针、US 布局 ASCII、手动 IPv4。任一正面按键停止输入。
`DONE` 是 HID 报告结果，目标操作是否成功需观察手机。不自动重放输入。

USB 等部分实现来源尚待核实。现有 Apache-2.0 声明保留，不能据此证明每个文件独立生成。
公开发行前解决[已记录的来源缺口](THIRD_PARTY.zh-CN.md)；设备验证与来源处理是两项门槛。

## 项目对比与致谢

感谢 [ja1chen/actell](https://github.com/ja1chen/actell) 和
[onehumanbeing/esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent)
对手机 Agent 与外接输入设备的探索。下表对比本固件配合 Anan 的方案，以及链接所指的上游版本。

| 项目 | 屏幕采集 | Agent 运行位置 | 命令通路 | iPhone 输入方式 |
| --- | --- | --- | --- | --- |
| 本固件 + Anan | 手机内 ScreenCaptureKit | iPhone | USB 以太网 / ANAN1 TCP → StopWatch | USB 绝对坐标鼠标与键盘 |
| [actell](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904) | CoreDevice 显示流 | iPhone | VPN 回环，或 ESP32-S3 USB NCM 转发 → CoreDevice | CoreDevice 点击与拖动 |
| [esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18) | AirPlay → 电脑端 UxPlay | 电脑 / Python | Wi-Fi TCP → ESP32-S3 | USB 相对鼠标与键盘 |

actell 还保留较早的 ScreenCaptureKit + USB NCM/HID 方案，该路线是本固件相关参考；
表格描述的是其后来的 CoreDevice 路线。esp32-iphone-agent 提供了外接 USB HID/Agent
架构参考。准确参考版本、实现来源待核实项和适用声明见 [THIRD_PARTY.md](THIRD_PARTY.md)。
