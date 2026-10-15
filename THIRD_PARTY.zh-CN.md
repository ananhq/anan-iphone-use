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

迁移表保留原始路径。本次目录整理将 `board.c` / `panel_init.h` 放入
`main/hardware/`，USB/HID 代码放入 `main/input/`，不改变实现/头文件字节。
第三方引擎、传感器和二维码源码位置保持不变，见[模块索引](main/README.zh-CN.md)。

**actell**，commit `e701786d1686823f9ab60524e689e387b7c3b904`：
[仓库](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904)、[来源声明](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/THIRD_PARTY.md)、[USB network](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_network.c)、[USB HID](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_hid.c)。
该版本组合发行声明 GPL-3.0-only，同时保留文件各自条款：原 v0.1/新独立文件头为 GPL-3.0-or-later，raw mux 模块为 GPL-2.0-only OR GPL-3.0-only，本次未导入这些模块，核实衍生关系时保留实际适用条款；这两个 USB 文件未含单独 SPDX，不表示可以重新许可。相似处包括 pbuf 复制/同步发送/free、50ms 超时和1514 字节上限、lwIP 网卡初始化顺序、2秒信号量初始化、eFuse 派生 MAC 与 host 末位翻转、NCM端点布局、HID 报告 ID/描述、USB事件/配置回调。Anan 在动作 worker、供电/按钮、协议和部分 USB 细节不同，但差异不能解决实现来源。本次迁移没有新复制 actell 文件，不发布“全部独立实现”保证。发布前核实实际衍生复用并保留对应许可/源码义务；项目整体已确定为 GPL-3.0-only，仅 credit 不能解决逐文件来源映射。

**esp32-iphone-agent**，commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`：
[仓库](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18)、[MIT许可](https://github.com/onehumanbeing/esp32-iphone-agent/blob/68584cfc8b3f227d6324ec7c8008c48b53f20a18/LICENSE)，Copyright 2026 Henry Yuan。
外接 USB HID/Agent 架构参考；其 Arduino/Wi-Fi/电脑端 Agent 不是构建依赖，本次未导入文件。历史复制/改写未穷尽审计；已保留[完整上游 MIT 声明](licenses/esp32-iphone-agent-MIT.txt)及 Copyright (c) 2026 Henry Yuan，逐文件复用仍待核实。

**M5Stack**：[StopWatch 文档](https://docs.m5stack.com/zh_CN/core/StopWatch)和其中原理图提供板级/下载模式依据；未导入厂商库或美术。产品页不能单独证明 M5PM1 寄存器行为；先保留原实现，独立来源保证前核对准确代码/参考。厂商资料不归本仓库许可证。

已撤回的 0.9.7 PSRAM 配置依据官方 ESP32-S3R8/8MB 硬件规格，以及出厂
[sdkconfig.defaults](https://github.com/m5stack/M5StopWatch-UserDemo/blob/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/sdkconfig.defaults)
中的 Octal、80MHz，固定提交 `6b4aa125288b6fe9dca661f10159f6e1e5ee785c`。
这些是配置事实，不导入出厂实现；分配器/PSRAM 实现来自既有固定 ESP-IDF5.5.5，
沿用其已保留声明。

## 构建依赖

| 依赖 | 固定来源/版本 | 保留声明 |
| --- | --- | --- |
| ESP-IDF | v5.5.5、`b774170ff46c393eeb5e495ea37936038d3f4f4f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt)，各组件声明仍适用 |
| esp_tinyusb | registry 2.2.0，hash 见锁文件 | [Apache-2.0](licenses/esp_tinyusb-Apache-2.0.txt) |
| TinyUSB | registry 0.21.0~2，hash 见锁文件 | [MIT](licenses/TinyUSB-MIT.txt)，Copyright 2012–2026 hathach |
| lwIP | IDF 子模块 `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` | [BSD声明](licenses/lwIP-BSD.txt)，Swedish Institute of Computer Science |

依赖由 IDF/registry 另行获取，不提交源码。这些许可副本不是所有已链接 SDK 组件的完整二进制许可清单；公开镜像前检查实际链接组件，随发行补齐适用版权/许可声明，保留 [LICENSE](LICENSE)/[NOTICE](NOTICE)，写入最终仓库地址/版本，并解决首方来源待核实项。原迁移未升级依赖或改变项目许可证；后续 GPL-3.0-only 决定在上方单独记录。

## MCP 开发新增内容

前表描述迁移历史。MCP 开发现将原动作例程提取至 `main/input/actions.c`，在 `main/main.c`
接入互斥，给 `board.c` 增加 I2C 总线访问接口；`usb.c` 与 PROTOCOL.md 保持字节不变。
原 USB/供电来源缺口仍未解决。

MCP、身份、按键 UI、条带渲染、工具 schema 和手机设置页是使用固定 SDK API 的首方
实现，没有导入 Anan 客户端或 Espressif MCP C SDK 代码；ASCII 字形自写，后加中文子集按下文记录由 OFL 字体派生。唯一改编的厂商
数据文件为 `main/hardware/panel_init.h`，命令来自 M5Stack
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

显示启动修正：`main/hardware/display.c` 现使用原生 IDF API 单独实现 M5IOE1 唤醒/重试/处理间隔，
显示供电前关闭扩展芯片 I2C 休眠。行为/寄存器事实核对上述固定 M5IOE1 commit 的
`src/M5IOE1.cpp`、`src/M5IOE1_i2c_compat.h`、`src/M5IOE1.h`，以及固定出厂 commit
的 `main/hal/hal_ioe.cpp`；未导入厂商库。保留[M5IOE1 完整 MIT 声明](licenses/M5IOE1-MIT.txt)，
Copyright (c) 2026 M5Stack Technology CO LTD。此修正不解决原 USB/供电来源缺口。

## 触屏/双语 UI 新增内容

`main/hardware/touch.c` 是有界原生 IDF 首方适配，寄存器/复位事实来自固定出厂版本的
`main/hal/drivers/cst820/cst820.cpp/.h`、`hal_display.cpp`、`hal_ioe.cpp`。
CST 文件没有独立 SPDX，已检查根目录 MIT 适用并保留完整 M5Stack 声明；不复制/
导入厂商 C++ 类、LVGL 或 M5GFX 实现。

`main/ui/ui_font.h` 为重新命名的 **StopWatch UI Glyphs**，Noto Sans SC 22px、weight500
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

## HID 扩展增量

候选 `0.5.0-mcp` 新增第一方有界解析/执行、schema 与离线验收场景；场景在实施前编写，模拟报告/时间，不代表真机验收。USB 使用既有锁定 TinyUSB `0.21.0~2` 的 Consumer 模板/API，没有复制上游实现或升级依赖。Consumer 数值依据 Apple 2026-09-21 配件指南第14章表14-3（包括 SDK 无命名枚举的0x029D），[调研](docs/research/hid-capabilities.zh-CN.md)记录PDF哈希。原 USB/板级来源缺口与许可声明继续保留；原鼠标/键盘ID及USB配置保留，追加Consumer ID3。

候选 `0.5.1-mcp` 合并工具发现与第一方参数解析，保留执行器及USB代码。新增合并验收场景在本轮实现前编写。不引入外部代码、不改变依赖或许可；旧来源缺口仍保留。

## 无线候选新增内容

候选 `0.6.0-wireless` 启用现有 ESP-IDF v5.5.5 锁内组件；不升级 SDK、不导入
Arduino 库、上游应用代码或厂商 HID demo。`input.c`、`ble_input.c`、
`ble_reports.c/h`、`wireless.c/h` 为使用 SDK API 的第一方实现。BLE 使用既有
TinyUSB 描述符宏；USB 描述符/发送函数体只作后端符号重命名，不改变项目许可。

| 启用组件 | 准确来源 | 声明 |
| --- | --- | --- |
| NimBLE | IDF 子模块 `685675c0128deafdd201c9eb82e61d227364646c` | [完整 LICENSE](licenses/NimBLE-LICENSE.txt)、[NOTICE](licenses/NimBLE-NOTICE.txt)、[BSD 声明](licenses/NimBLE-BSD-NOTICES.txt) |
| TinyCrypt / micro-ecc | NimBLE `ext/tinycrypt` 与锁定 SDK 的 `components/bt/common/tinycrypt` | [完整 BSD 声明](licenses/TinyCrypt-BSD.txt)，保留链接文件实际条款 |
| Wi-Fi 射频 | `22bb3517b08760ed43b889013f8a122db0d46da0` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| PHY | `59c1234e929212aec0fdda75769b759951235536` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| 共存 | `79e618f033e926d134decc6d95bb1b1850bd032c` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |
| S3 蓝牙控制器（c3-family 库） | `0a08c4b32f3666003080b662a1a61794da24ff0f` | [Apache-2.0](licenses/ESP-IDF-Apache-2.0.txt) |

四个射频 LICENSE 与已保留 Apache-2.0 文件完全一致，SHA-256 为
`cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30`。SDK
射频库为预编译组件，不声称其实现源码可得。NimBLE LICENSE 还列出平台特定
材料，S3 镜像不是 Nordic/babblesim 目标；发布二进制仍需最终链接文件声明清单。

参考 [xhoantran/iphone-use](https://github.com/xhoantran/iphone-use/tree/bc0fe36e78ec5a3af095c813574e3a8b7bbea4dd)
的 BLE 绝对定位后发送相对鼠标按键思路，不复制 Swift 文件或报告字节，不引入
AirPlay/客户端代码。原 USB 来源缺口保持未解决。见[无线范围](docs/history/wireless-development.zh-CN.md)。

`0.6.1-wifi-scan` 使用同一锁定 `esp_wifi.h` API 增加第一方扫描调度与配网选择
界面，不导入外部源码，不改变依赖、报告描述符或许可。

候选 `0.7.0-wireless-setup` 增加第一方网络存储、HTTP Host/Origin 和发现适配器。CMake 启用已锁定 lwIP 子模块 `fd432e4ee2cfb7f7f1c7eb7227e0173412e7b84e` 内的 `src/apps/mdns/mdns.c`、`mdns_out.c`、`mdns_domain.c`，不修改或复制到仓库；其 Copyright (c) 2015 Verisure Innovation AB BSD 文件头保留于[完整 mDNS 通知](licenses/lwIP-mDNS-BSD.txt)，与既有 [lwIP 通知](licenses/lwIP-BSD.txt)并列保留。全局 responder 定义及 SDK client-data 槽位保证 netif ABI 一致。不增加新依赖、升级 SDK、引入上游应用或更改许可证，SoftAP/DHCP 使用既有锁定 SDK 组件。

候选 `0.7.1-mdns` 编译同一锁定 `src/apps/mdns/mdns.c` 的具名构建生成适配版本，原文件 SHA-256 `2bd12a3bb8b3c8830c56c2b5c3b8073ab5f71654171602210f63cf6dafc8d5e9`。根 CMake 核对精确哈希，仅改变 IPv6 探测条件：link-local 地址槽 0 无效时跳过该地址族，保留 IPv4 探测/冲突检查。SDK 检出保持不变，生成源码留在忽略的 build/；保留原 Verisure 文件头及[完整 BSD 通知](licenses/lwIP-mDNS-BSD.txt)，不升级 SDK/依赖或改变项目许可。

热点二维码仅原样引入 Nayuki **v1.8.0** 的 `c/qrcodegen.c`、`c/qrcodegen.h`，提交 `7ad95cedd8464a87f82221283612732ae4f3f305`，[上游](https://github.com/nayuki/QR-Code-generator/tree/7ad95cedd8464a87f82221283612732ae4f3f305/c)，MIT，Copyright (c) Project Nayuki。保留完整许可文件头与 [MIT 通知](licenses/Nayuki-QR-MIT.txt)，第一方适配/绘制独立。SHA-256：C `300eff07ee25baaa7578f20284411638154716379437391e7e689c0e6ce81403`，头文件 `e82df4bff37d18b5863b9e7486fe6bda1b6cda8c3b9ecebfec473907265cb589`。不升级 SDK 或改变项目许可。ZXing-C++ 2.3.0 仅为可选独立离线解码检查工具，使用合成数据，不链接或复制进固件。

## 大按钮界面与电量显示

候选 `0.7.6-touch-ui` 增加第一方菜单/布局/触摸反馈和粗略电压电量估算。
M5PM1 的电压寄存器事实（0x22/0x23，小端毫伏）核对自
[M5PM1 commit be9a5456c007c333e7ac963f33bfde1ffa5d82ee](https://github.com/m5stack/M5PM1/tree/be9a5456c007c333e7ac963f33bfde1ffa5d82ee)
的 `docs/M5PM1_Datasheet_EN.pdf` 与 `src/M5PM1.h/.cpp`。未复制/链接厂商库、
类或电量算法，保留[完整 M5PM1 MIT 声明](licenses/M5PM1-MIT.txt)，
Copyright (c) 2025 M5Stack Technology CO LTD。使用相同固定来源的 OFL 字体，
为新文案重新生成字形子集；运行依赖和供电策略不变。原有板级/USB 来源缺口保留。

`0.7.8-eyes` 的采样、椭圆分条绘制与离线预览均为第一方程序化形状，不引入外部
美术素材、动画库或运行依赖；现有字体声明和来源缺口不变。


## 可交互表情 / BMI270

设计参考：[Trentct/m5stack-stopwatch-avatar](https://github.com/Trentct/m5stack-stopwatch-avatar/tree/204963257cb4dc2f3d7501eff900897bac55ef82)，AGPL-3.0-or-later。仅借鉴姿态/交互/过渡/变化区域刷新的一般思路，不引入代码、表情表或美术素材。原生实现为第一方，项目许可证保留。

新增传感器来源：[Bosch BMI270 SensorAPI](https://github.com/boschsensortec/BMI270_SensorAPI/tree/41129fcfe39c583ee5462d79195741945d51c1fe)，固定提交 `41129fcfe39c583ee5462d79195741945d51c1fe`，基础 BMI270 v2.86.1。以下五个文件含配置数组按字节原样复制，保留文件声明及[完整 BSD-3-Clause 声明](licenses/Bosch-BMI270-BSD-3-Clause.txt)，Copyright (c) 2023 Bosch Sensortec GmbH。第一方 ESP-IDF 适配与滤波独立。

| 文件 | 原始 SHA-256 |
| --- | --- |
| `main/vendor/bmi270/bmi2.c` | `8d42ba3281b8448f674a4df616a7e176a259ad3c38c53b6d2c49bfdbdb0a82db` |
| `main/vendor/bmi270/bmi2.h` | `a86c53c480974a0c56e24d9d4e736445c61ebc07addec6d3eaceddb8a2c97cd3` |
| `main/vendor/bmi270/bmi2_defs.h` | `39c2cf19bfbf735cb0ed209efd975a731be50dff71b432b24a6a2625b2b14304` |
| `main/vendor/bmi270/bmi270.c` | `f6446867378bbcc90ea97ef6593555de8767ca20b799355c1b37c2cabb7b7b41` |
| `main/vendor/bmi270/bmi270.h` | `7979ec4c943e2825351bd9d3d267b63abc6f8314c1f13f1c37e95cabc3fc82c8` |

## 上游表情引入（0.9.0）

维护者于 2026-10-15 同意引入 AGPL-3.0-or-later 代码。新控制表情替代 0.8.x
“仅设计参考”的实现。Trentct/m5stack-stopwatch-avatar 固定提交
`204963257cb4dc2f3d7501eff900897bac55ef82` 的两个引擎文件适配至
`main/vendor/stopwatch_avatar/`；保留[原始文件哈希与修改说明](main/vendor/stopwatch_avatar/ORIGIN.md)、
[上游第三方说明](main/vendor/stopwatch_avatar/UPSTREAM-NOTICES.md) 和
[完整 AGPL 文本](licenses/StopWatch-avatar-AGPL-3.0.txt)。原生绘图/事件适配为
AGPL-3.0-or-later；既有 GPL 文件保留许可，组合依 GPLv3/AGPLv3 第 13 节处理，
包括 `/source` 网络源码提供，见[源码与构建](SOURCE.zh-CN.md)。不链接上游
Arduino/M5Unified/M5GFX 硬件栈。既有来源缺口和链接 SDK 通知审查限制仍然保留。

已同意的 0.9.2 跟手修正仅修改上游 `updateInteraction` 的运动积分，记录在 [ORIGIN](main/vendor/stopwatch_avatar/ORIGIN.md)；表情表、眨眼/时间轴与投影保留。紧凑绘制及较短的仅 MCP 输入反馈为本地 AGPL 适配，没有新增第三方源码/依赖或许可变更。

0.9.3 紧凑窗口修正为 `main/hardware/display.c` 中的首方实现。偶数坐标/尺寸要求与
[Zephyr CO5300 驱动](https://github.com/zephyrproject-rtos/zephyr/blob/2afc59f6d9784ad7013242d5477cba9745dde484/drivers/display/display_co5300.c#L125)
（提交 `2afc59f6d9784ad7013242d5477cba9745dde484`）交叉核对；未复制/导入源码，
未新增依赖或改变许可。
