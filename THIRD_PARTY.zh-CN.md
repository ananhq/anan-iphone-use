# 来源与第三方声明

[English](THIRD_PARTY.md) · 简体中文

Anan iPhone Use（`anan-iphone-use`）为选定项目名称。用户确认参考了 actell 源码，USB 的逐文件衍生关系仍待核实。维护者已授权选择开放的项目许可；项目整体现采用与 actell 参考来源相容的 GPL-3.0-only，见 [LICENSE](LICENSE)。保留文件原声明及第三方条款，旧根 Apache-2.0 文本保留于[历史声明](licenses/Legacy-first-party-Apache-2.0.txt)。见[许可决定与来源处理](docs/license-plan.zh-CN.md)。此决定不表示衍生审计完成，也不改变依赖许可。

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
该版本组合发行声明 GPL-3.0-only，同时保留文件各自条款：原 v0.1/新独立文件头为 GPL-3.0-or-later，raw mux 模块为 GPL-2.0-only OR GPL-3.0-only，本次未导入这些模块，核实衍生关系时保留实际适用条款；这两个 USB 文件未含单独 SPDX，不表示可以重新许可。相似处包括 pbuf 复制/同步发送/free、50ms 超时和1514 字节上限、lwIP 网卡初始化顺序、2秒信号量初始化、eFuse 派生 MAC 与 host 末位翻转、NCM端点布局、HID 报告 ID/描述、USB事件/配置回调。Anan 在动作 worker、供电/按钮、协议和部分 USB 细节不同，但差异不能解决实现来源。本次迁移没有新复制 actell 文件，不发布“全部独立实现”保证。发布前核实实际衍生复用并保留对应许可/源码义务；项目整体已确定为 GPL-3.0-only，仅 credit 不能解决逐文件来源映射。

**esp32-iphone-agent**，commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`：
[仓库](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18)、[MIT许可](https://github.com/onehumanbeing/esp32-iphone-agent/blob/68584cfc8b3f227d6324ec7c8008c48b53f20a18/LICENSE)，Copyright 2026 Henry Yuan。
外接 USB HID/Agent 架构参考；其 Arduino/Wi-Fi/电脑端 Agent 不是构建依赖，本次未导入文件。历史复制/改写未穷尽审计；已保留[完整上游 MIT 声明](licenses/esp32-iphone-agent-MIT.txt)及 Copyright (c) 2026 Henry Yuan，逐文件复用仍待核实。

**M5Stack**：[StopWatch 文档](https://docs.m5stack.com/zh_CN/core/StopWatch)和其中原理图提供板级/下载模式依据；未导入厂商库或美术。产品页不能单独证明 M5PM1 寄存器行为；先保留原实现，独立来源保证前核对准确代码/参考。厂商资料不归本仓库许可证。

## 构建依赖

| 依赖 | 固定来源/版本 | 保留声明 |
| --- | --- | --- |
| ESP-IDF | v5.5.5、`b774170ff46c393eeb5e495ea37936038d3f4f4f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt)，各组件声明仍适用 |
| esp_tinyusb | registry 2.2.0，hash 见锁文件 | [Apache-2.0](licenses/esp_tinyusb-Apache-2.0.txt) |
| TinyUSB | registry 0.21.0~2，hash 见锁文件 | [MIT](licenses/TinyUSB-MIT.txt)，Copyright 2012–2026 hathach |
| lwIP | IDF 子模块 `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` | [BSD声明](licenses/lwIP-BSD.txt)，Swedish Institute of Computer Science |

依赖由 IDF/registry 另行获取，不提交源码。这些许可副本不是所有已链接 SDK 组件的完整二进制许可清单；公开镜像前检查实际链接组件，随发行补齐适用版权/许可声明，保留 [LICENSE](LICENSE)/[NOTICE](NOTICE)，写入最终仓库地址/版本，并解决首方来源待核实项。原迁移未升级依赖或改变项目许可证；后续 GPL-3.0-only 决定在上方单独记录。

## MCP 开发新增内容

前表描述迁移历史。MCP 开发现将原动作例程提取至 `main/actions.c`，在 `main/main.c`
接入互斥，给 `board.c` 增加 I2C 总线访问接口；`usb.c` 与 PROTOCOL.md 保持字节不变。
原 USB/供电来源缺口仍未解决。

MCP、身份、按键 UI、条带渲染、工具 schema 和手机设置页是使用固定 SDK API 的首方
实现，没有导入 Anan 客户端或 Espressif MCP C SDK 代码；ASCII 字形自写，后加中文子集按下文记录由 OFL 字体派生。唯一改编的厂商
数据文件为 `main/panel_init.h`，命令来自 M5Stack
[M5StopWatch-UserDemo main/hal/hal_display.cpp](https://github.com/m5stack/M5StopWatch-UserDemo/blob/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/main/hal/hal_display.cpp)，
commit `6b4aa125288b6fe9dca661f10159f6e1e5ee785c`，MIT，Copyright (c) 2026
M5Stack Technology CO LTD，[完整声明](licenses/M5Stack-demo-MIT.txt)。QSPI 偏移及
IO 扩展引脚事实参考同一出厂项目及 [M5IOE1 定义](https://github.com/m5stack/M5IOE1/tree/846eec7d05e25c09013be2acdb8804487f48a62e)
（commit `846eec7d05e25c09013be2acdb8804487f48a62e`），没有复制厂商 UI/库。

| 新增使用 | 准确来源 | 声明 |
| --- | --- | --- |
| CO5300 驱动 | Registry `espressif/esp_lcd_co5300` 2.2.0，组件哈希见锁文件 | [Apache-2.0](licenses/esp_lcd_co5300-Apache-2.0.txt)，获取源码保留 Espressif 各文件声明 |
| 构建辅助 | Registry `espressif/cmake_utilities` 0.5.3，间接锁定 | [Apache-2.0](licenses/cmake_utilities-Apache-2.0.txt) |
| JSON | IDF cJSON1.7.19，`b2890c8d76bbb64e710585ebc0a917196b9c67e7` | [MIT](licenses/cJSON-MIT.txt)，Dave Gamble 及贡献者 |
| TLS/X.509/DRBG | IDF mbedTLS3.6.6，`9d669eadb1955d348986b9280156710aaaadf79f` | 选用 Apache-2.0，[完整上游双许可文本](licenses/mbedTLS-dual-license.txt)保留 |
| HTTP/HTTPS/NVS/SPI/LCD/熵 | 固定 ESP-IDF5.5.5 内置组件 | 已有 IDF 声明及适用组件/文件声明 |

原 MCP 新增阶段未改变整体许可证或解决来源问题；后续整体 GPL-3.0-only 决定不消除来源缺口。生成的私钥、证书、token 和配对记录
不提交。公开镜像前仍需按前述要求补完整链接组件声明清单。

显示启动修正：`main/display.c` 现使用原生 IDF API 单独实现 M5IOE1 唤醒/重试/处理间隔，
显示供电前关闭扩展芯片 I2C 休眠。行为/寄存器事实核对上述固定 M5IOE1 commit 的
`src/M5IOE1.cpp`、`src/M5IOE1_i2c_compat.h`、`src/M5IOE1.h`，以及固定出厂 commit
的 `main/hal/hal_ioe.cpp`；未导入厂商库。保留[M5IOE1 完整 MIT 声明](licenses/M5IOE1-MIT.txt)，
Copyright (c) 2026 M5Stack Technology CO LTD。此修正不解决原 USB/供电来源缺口。

## 触屏/双语 UI 新增内容

`main/touch.c` 是有界原生 IDF 首方适配，寄存器/复位事实来自固定出厂版本的
`main/hal/drivers/cst820/cst820.cpp/.h`、`hal_display.cpp`、`hal_ioe.cpp`。
CST 文件没有独立 SPDX，已检查根目录 MIT 适用并保留完整 M5Stack 声明；不复制/
导入厂商 C++ 类、LVGL 或 M5GFX 实现。

`main/ui_font.h` 为重新命名的 **StopWatch UI Glyphs**，Noto Sans SC 22px、weight500
栅格化后生成 24px 字形位图子集。准确来源：google/fonts commit
`a85815a42757630ce188fdad368c2dfc444d4773`、
`ofl/notosanssc/NotoSansSC[wght].ttf`、Git blob
`fb0637bafbcd804fe32152370a1225990745b4bc`、SHA-256
`a3041811a78c361b1de50f953c805e0244951c21c5bd412f7232ef0d899af0da`。
17,772,300 字节原字体不放入仓库/固件，只含 UI 所需字形。
[完整 SIL OFL1.1](licenses/NotoSansSC-OFL-1.1.txt)，Copyright 2014–2021 Adobe，
Reserved Font Name 'Source'。生成字形数据仍为 OFL-1.1，不改为 Apache-2.0；自写
ASCII 字形/绘制/布局/手势代码保留首方声明，整体 GPL-3.0-only 决定后，这些文件各自许可和来源缺口仍保留。

`tools/generate_ui_font.py`、`tools/render_ui.py` 仅用 Pillow12.3.0 在构建时生成字形/
静态文档预览，不链接进固件。绘制使用 Pillow 的 FreeType 支持；生成前检查原字体
哈希，预览导出相同的首方 C 按钮布局。若另行发行开发工具，保留工具链适用声明。
