# MCP 开发决策

[English](mcp-development.md) · 简体中文

用户授权优先 MCP、其次无线，需要时采用简单按钮/选项配置 UI，不改 Anan；接受首次
在 iPhone 安装/信任设备证书，随后明确要求开始实现。MCP 开发分支现已实现代码，
不宣称设备/互通验收通过。检查点 `4c6015c` 可恢复，其来源与运行验收缺口继续保留。

## 原首阶段决策（历史）

- MCP2025-11-25，USB HTTPS8443 上仅工具的 Streamable HTTP JSON POST，
  GET405、取消和终止会话。使用 IDF HTTP/HTTPS、cJSON、mbedTLS API，不引入完整
  MCP C SDK 或 Agent runtime。
- 六个工具复用原 HID 执行，默认仍是旧 USB；设备选择 MCP、物理授权持续至停止/拔线/重启、单会话归属。
- 手机 HTTP 配对与设备验证码确认，独立 CA/证书/token、正常 iPhone 信任、仅 HTTPS
  导出密钥、独立身份分区；无共用签名私钥或公网中继，普通升级可保留身份。
- 原生 CO5300/按钮状态 UI，固定驱动2.2.0，保留声明的 MIT 厂商命令表、自写字形。
  不引入 LVGL、触摸或模型思考显示。
- ANAN1 原契约、USB 描述符及报告时序保留；不改变来源结论或整体许可证，新增声明单列。

已检查 Anan HTTPS guard、Swift MCP SDK0.12.1 支持选定版本/JSON 响应形态，GET405
行为仅有源码证据。原生 `phone.observe` 与板上 MCP 工具分开，保留 USB/采屏/归属
门槛。MCP 模式保留闲置 ANAN1 READY/PING，旧输入拒绝。用户暂缓进一步观察开发，
不宣称板上截图或 Anan 无线观察。具体实现、限制、按键和配对见[设备 MCP](mcp.zh-CN.md)。

## 无线延后

候选路线为 Wi-Fi STA2.4GHz 承载 MCP、BLE HID 输入，保留 USB。手机设置页填写凭据，
设备仅用选项/按钮。首个可行性门槛是 iPhone 是否接受所需 BLE 绝对坐标报告，IDF
相对鼠标示例不足以证明。Wi-Fi/BLE、续航、TLS 名称及重连/无重放需单独实现及设备
验收，不把本 MCP 阶段当成无线支持。

## 实现前定义的检查

| 场景 | 所需证据 |
| --- | --- |
| 静态/构建 | 固定 IDF 编译打包、RAM/Flash、依赖/来源声明，源码/镜像输入无秘密 |
| 发现 | 用兼容独立客户端执行 initialize/版本/initialized/tools-list；不改代码的 Anan 单独验证 |
| 输入 | 合成竖屏笔记，单次点击、拖动、按键、ASCII；观察可见结果并保留失败 |
| 停止 | 输入中按键/stop/取消/拔线；测量延迟，主机仍可用时释放报告 |
| 仲裁 | 旧协议/MCP 同时尝试，单执行器、无重复输入、无隐式模式切换/重放 |
| 异常请求 | 长度/嵌套/类型/版本/Origin/auth 在派发前拒绝，诊断不含内容或凭据 |
| 登记 | 物理确认、描述文件信任和正常 TLS；显式重置后旧凭据失效 |
| 旧模式回归 | 默认 USB 的 ANAN1 问候、心跳、ID 排序、报告时序、停止和无重放 |

设备、真实模型、模拟器和 E2E 执行需另外授权当前范围，不在实现后补单元测试。
未观察前不将发现/互操作、显示、停止延迟或硬件验收标为通过。
无线排在 MCP 之后，其输入可行性的首个门槛仍为 BLE 绝对坐标兼容性。

## 已检查资料

- [MCP 2025-11-25 传输](https://modelcontextprotocol.io/specification/2025-11-25/basic/transports)、[生命周期](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle)。
- [Swift SDK 0.12.1](https://github.com/modelcontextprotocol/swift-sdk/tree/a0ae212ebf6eab5f754c3129608bc5557637e605)；Anan 本地 MCP/Phone 源码只读未改。
- [ESP-IDF v5.5.5 HTTPS](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/protocols/esp_https_server.html)、[ESP32-S3 熵源](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32s3/api-reference/system/random.html)，并核对固定 SDK 头文件。
- [Apple 描述文件信任](https://support.apple.com/en-us/102390)、[TLS 证书要求](https://support.apple.com/en-us/103769)。
- [Espressif MCP SDK 参考](https://github.com/espressif/esp-iot-solution/tree/58c2f852125cb1606c2137fc9b697efc3ce1c3d0/components/mcp-c-sdk)：manifest 为 2.0.1、IDF ≥5.4、Apache-2.0；检查但未导入。
- [StopWatch 硬件](https://docs.m5stack.com/en/core/StopWatch)、[出厂显示/IO 参考](https://github.com/m5stack/M5StopWatch-UserDemo/tree/6b4aa125288b6fe9dca661f10159f6e1e5ee785c/main/hal)、[M5IOE1 定义](https://github.com/m5stack/M5IOE1/tree/846eec7d05e25c09013be2acdb8804487f48a62e)：源文件声明 MIT，本次调研仅改编并保留声明的显示命令表。
- [CO5300 驱动](https://components.espressif.com/components/espressif/esp_lcd_co5300/versions/2.2.0/readme)。

## HTTP 配置后续：已约定范围与检查

用户选择当前 USB 连接使用明文 HTTP，要求等待 Anan 支持后再改固件，现反馈
客户端已就绪。将原 MCP handler 接到 HTTP80 `/mcp`，物理确认后的 `/config`
导出 HTTP 地址，同一网页完成配对与导入，不再要求安装证书。保留可选 HTTPS8443、
独立身份/token、仅 USB 可达、六个工具、授权/停止/会话和 ANAN1 行为。不修改
Anan、不移除既有来源/许可整理，也不加入无线传输。

编码前定义编译/打包、源码不变量审查、双语静态 UI 预览边界/字形、内嵌
JavaScript/配置 JSON 与本地链接检查。不在实现后补单元测试，不执行设备输入。
用户自行烧录，仅移除旧 StopWatch CA 描述文件/连接配置，重置板上配对后重复
新用户流程；与静态检查分别记录，不整片擦除 Flash 或清除无关手机设置。

## 仅 MCP 后续：已批准范围与检查

用户反馈 HTTP 新用户流程验证良好，并明确要求删除旧 TCP，仅保留 MCP。
移除28766端口、ANAN1解析、READY/PING/STOP与旧输入归属；开机直接进入MCP，
移除控制来源切换按钮，保留USB NCM/HID、配对、六个工具、独立身份、语言记忆、
不持久化授权、输入时序与立即停止。不修改 Anan；已检查的观察就绪条件允许
未连接输入时采屏，旧 Anan 直连输入将不再能连接。

实现前定义编译/打包、已退役服务的源码/链接符号审查、USB/动作/身份哈希保留、
双语静态 UI 字形/布局预览、JSON/JavaScript/Python语法及文档链接检查。
PROTOCOL.md、检查点和旧构建证据保留为历史记录，与当前 MCP 说明区分。
不在实现后补单元测试，不执行设备/手机动作；新镜像需用户重新烧录验收，
不涉及公开发布或许可变更。
