# 来源与第三方声明

[English](THIRD_PARTY.md) · 简体中文

## 迁移范围与证据

2026-10-13 从 Anan 的**未跟踪** `hardware/stopwatch/` 提取。[source-snapshot.json](artifacts/migration/source-snapshot.json) 记录 Anan HEAD、每个原文件哈希/状态、历史镜像及源码 manifest。Anan HEAD 单独不能标识这些未提交内容。迁移保留原声明，不追溯证明每个文件如何生成。

| 原文件 | 声明 / 来源处理 |
| --- | --- |
| `main/main.c`、`main/bridge.h` | 原 Apache-2.0 SPDX；ANAN1 解析/worker/API 字节不变，未审计生成记录，不保证独立来源 |
| `main/board.c` | 原 Apache-2.0 SPDX；板级引脚/供电事实对照 M5Stack 资料/原理图，寄存器实现的准确来源待核实 |
| `main/usb.c` | 原 Apache-2.0 SPDX；字节不变，与下列 actell 文件有较多结构相似，复制/翻译/改写或 API 常见模式来源尚未确定 |
| `main/CMakeLists.txt`、`main/idf_component.yml`、`CMakeLists.txt`、`sdkconfig.defaults`、`dependencies.lock` | 其他原构建配置字节不变；根 CMake 固定原 IDF 应用版本 `ec6bb29-dirty`，避免从新 Git 推导；保留 Anan 项目声明、记录依赖版本/哈希 |
| `build.sh`、`flash.sh`、`package.py`、`.gitignore` | 保留原脚本/忽略规则；`.gitignore` 补本地编辑器/缓存排除，未完成外部实现来源审计 |
| `README.md`、`README.zh-CN.md`、`PROTOCOL.md` | 快照保留原哈希；重整文档/路径/来源措辞，不改变协议行为 |
| 新增文档/记录 | 本次迁移编写，标明事实及上游链接，不新增上游实现代码 |

## 已检查的参考

**actell**，commit `e701786d1686823f9ab60524e689e387b7c3b904`：
[仓库](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904)、[来源声明](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/THIRD_PARTY.md)、[USB network](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_network.c)、[USB HID](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_hid.c)。
该版本组合发行声明 GPL-3.0-only，同时保留文件各自条款；这两个 USB 文件未含单独 SPDX，不表示可以重新许可。相似处包括 pbuf 复制/同步发送/free、50ms 超时和1514 字节上限、lwIP 网卡初始化顺序、2秒信号量初始化、eFuse 派生 MAC 与 host 末位翻转、NCM端点布局、HID 报告 ID/描述、USB事件/配置回调。Anan 在动作 worker、供电/按钮、协议和部分 USB 细节不同，但差异不能解决实现来源。本次迁移没有新复制 actell 文件，不发布“全部独立实现”保证。若确认衍生复用，发布前处理对应许可/源码义务，改变项目许可证需用户确认；仅 credit 不解决该问题。

**esp32-iphone-agent**，commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`：
[仓库](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18)、[MIT许可](https://github.com/onehumanbeing/esp32-iphone-agent/blob/68584cfc8b3f227d6324ec7c8008c48b53f20a18/LICENSE)，Copyright 2026 Henry Yuan。
外接 USB HID/Agent 架构参考；其 Arduino/Wi-Fi/电脑端 Agent 不是构建依赖，本次未导入文件。历史复制/改写未穷尽审计，若确认复用需保留上游完整声明。

**M5Stack**：[StopWatch 文档](https://docs.m5stack.com/zh_CN/core/StopWatch)和其中原理图提供板级/下载模式依据；未导入厂商库或美术。产品页不能单独证明 M5PM1 寄存器行为；先保留原实现，独立来源保证前核对准确代码/参考。厂商资料不归本仓库许可证。

## 构建依赖

| 依赖 | 固定来源/版本 | 保留声明 |
| --- | --- | --- |
| ESP-IDF | v5.5.5、`b774170ff46c393eeb5e495ea37936038d3f4f4f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt)，各组件声明仍适用 |
| esp_tinyusb | registry 2.2.0，hash 见锁文件 | [Apache-2.0](licenses/esp_tinyusb-Apache-2.0.txt) |
| TinyUSB | registry 0.21.0~2，hash 见锁文件 | [MIT](licenses/TinyUSB-MIT.txt)，Copyright 2012–2026 hathach |
| lwIP | IDF 子模块 `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` | [BSD声明](licenses/lwIP-BSD.txt)，Swedish Institute of Computer Science |

依赖由 IDF/registry 另行获取，不提交源码。这些许可副本不是所有已链接 SDK 组件的完整二进制许可清单；公开镜像前检查实际链接组件，随发行补齐适用版权/许可声明，保留 [LICENSE](LICENSE)/[NOTICE](NOTICE)，写入最终仓库地址/版本，并解决首方来源待核实项。本次不升级依赖、不改变项目许可证。
