# Anan iPhone Use：许可与来源处理

[English](license-plan.md) · 简体中文

**当前补充（0.9.0）：** 维护者于 2026-10-15 同意引入 AGPL-3.0-or-later 表情引擎。下方旧 GPL-only 结论描述既有代码/检查点；新组合固件依 GPLv3/AGPLv3 第 13 节处理并提供网络源码，逐文件许可保留。见[源码与构建](../SOURCE.zh-CN.md)及[来源记录](../main/vendor/stopwatch_avatar/ORIGIN.md)。

用户确认参考了 actell 源码，并授权选择开放的项目许可。Anan iPhone Use（`anan-iphone-use`）整体现采用 **GPL-3.0-only**，完整文本见 [LICENSE](../LICENSE)。旧根 Apache-2.0 文本保留于 [Legacy-first-party-Apache-2.0.txt](../licenses/Legacy-first-party-Apache-2.0.txt)。现有源码声明、第三方许可和历史证据保留。这是整体发行许可决定，不代表已完成来源审计或全部独立实现。

## 当前声明与来源处理

| 范围 | 当前声明 / 证据 | 所需处理 |
| --- | --- | --- |
| 根 LICENSE 与现有首方 C 文件 | 根 LICENSE 为 GPL-3.0，保留历史 Apache-2.0 文件声明及原许可授予；声明本身不能解决来源问题 | 整体按 GPL-3.0-only 发行并逐文件核对声明；保留原版权/许可授予和适用通知，不盲目替换所有头部 |
| `main/input/usb.c` | 现有 Apache-2.0 SPDX，与检查点相同；和 actell USB 文件有较多结构重合 | 核实具体复制/改写部分，保留适用 actell 条款与修改说明；不能认为当前 SPDX 已解决 GPL 衍生问题 |
| 原 `main/main.c`、`bridge.h`、`board.c` 及后续动作提取 | 保留原声明，历史生成及供电寄存器来源未完全建立 | 分别审查来源和修改，保留快照，公开前明确义务 |
| MCP、身份、显示/触摸等新增模块 | 首方代码使用固定 SDK API，MIT 面板命令数据、OFL 中文字形另有记录 | 审查组合发行及各项通知；项目整体 GPL 发行不能抹去依赖的 MIT/Apache/BSD/OFL 条款 |
| esp32-iphone-agent | MIT，Copyright (c) 2026 Henry Yuan | 保留[完整上游声明](../licenses/esp32-iphone-agent-MIT.txt)，明确历史实际复用范围后才能声称完成审计 |
| SDK 与库 | ESP-IDF/esp_tinyusb 为 Apache-2.0，TinyUSB 为 MIT，lwIP 为 BSD，其余通知见 THIRD_PARTY | 保留各自许可、准确版本和声明，补齐实际链接二进制清单 |

## 准确参考与修改记录

- actell：commit `e701786d1686823f9ab60524e689e387b7c3b904`，[根许可](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/LICENSE)、[来源通知](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/THIRD_PARTY.md)、[USB 网络](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_network.c)、[USB HID](https://github.com/ja1chen/actell/blob/e701786d1686823f9ab60524e689e387b7c3b904/firmware-device/main/usb_hid.c)。已检查发行声明为 GPL-3.0-only；上游 THIRD_PARTY 保留原 v0.1/新独立文件的 GPL-3.0-or-later 头部，以及 raw mux 的 GPL-2.0-only OR GPL-3.0-only，本次未导入这些模块，保留文件各自条款；两个 USB 文件没有独立 SPDX，不表示可重新许可。准确本地衍生关系仍待核实。
- 本地修改：迁移时保持 `main/input/usb.c` 字节不变，后续 MCP 开发也未改变它。本地协议/动作逻辑与 actell 不同，后续将动作提取到 `actions.c`，在 `main.c` 加互斥，导出板级 I2C 总线并增加 MCP/UI 模块。这些是本地修改事实，不是完整的上游至本地衍生映射。[来源快照](../artifacts/migration/source-snapshot.json)、检查点 `4c6015c` 和 [THIRD_PARTY](../THIRD_PARTY.zh-CN.md)保留详细记录。
- esp32-iphone-agent：commit `68584cfc8b3f227d6324ec7c8008c48b53f20a18`，原文件 `LICENSE`，MIT，Copyright (c) 2026 Henry Yuan。完整通知原样复制到 `licenses/esp32-iphone-agent-MIT.txt`；本次文档修改没有导入实现代码。

## GPL 发行所需处理

1. 整体发行许可已确定为 GPL-3.0-only，完成逐文件来源/通知映射，重点处理 USB 与原供电实现。保留准确上游版本、版权、条款和所需的明确修改声明。
2. 保留第三方许可与历史证据。credit 不能替代许可义务；改变根 LICENSE 本身不能完成来源审查。
3. 若二进制发行适用 GPLv3，按合规发行方式提供对应源码（Corresponding Source），包含所需源码及脚本/构建材料；适用时提供安装信息（Installation Information）。见 [GPLv3 第1、5、6节](https://www.gnu.org/licenses/gpl-3.0.html)和[发行检查](release-preparation.zh-CN.md)。
4. 记录实际发行的源码 commit、依赖/工具链版本和镜像哈希；公开前补齐应用/引导程序实际链接声明清单，不含凭据、配对记录、私人数据或生成缓存。

根许可决定已应用。本次许可/文档更新不修改源码实现、原文件头、依赖版本、历史产物，不公开发布或跨仓库修改 [Anan](https://anan.sh)；准确衍生审计与发行准备仍为独立工作。

仅 MCP 后续移除本地 ANAN1 TCP 解析/服务，不改变保留的 USB 实现，也不能
解决已记录的 actell 来源问题；整体许可已确定为 GPL-3.0-only，依赖声明保持原样。
