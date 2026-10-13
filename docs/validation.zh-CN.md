# 迁移验证记录

[English](validation.md) · 简体中文

## 已定义验收范围

本次授权本地提取、文档、编译和静态检查，不含烧录、手机/HID/模型操作、模拟器/E2E 或发布。编码前定义检查：原运行/构建输入 SHA-256 不变、在 Anan 外独立编译打包、核对目标/偏移/Flash/锁/源码 manifest、检查英中链接/忽略规则、记录来源缺口。Anan 的 A–H 真机场景不自动获得本轮授权。

[source-snapshot.json](../artifacts/migration/source-snapshot.json) 标识原 16 个文件；原 12 个构建输入在提取时全部匹配历史镜像 manifest。运行源码、脚本、锁和协议行为不变。根 CMake 固定原先由 Anan Git 推导的 IDF 应用版本 `ec6bb29-dirty`，这是唯一构建配置调整。因为 package.py 会 hash main/ 所有文件，新增模块 README 会成为额外 manifest 条目，但不会编译进源码。

## 证据层级

| 证据 | 状态 |
| --- | --- |
| 历史镜像 | 455840 字节，SHA-256 `5e3e96b3bf2e1699c6406874de66a85e7efa1e3823ec83738f4971b0f2d8e230`，提取时核对源码 hash |
| 历史烧录 | 用户提供 esptool 写入/hash 成功输出，不是本次迁移真机执行 |
| 历史行为 | 用户报告以太网、静态 IP、连接、预览、observe 和手机输入可用 |
| 完整任务/持续后台 | 备忘录任务困难；Anan 后台改进有用户反馈，缺少完整固件验收轨迹 |
| 迁移构建/静态 | 命令/工具版本/结果见 [build-check.json](../artifacts/migration/build-check.json)，不是设备验收 |
| 来源 | 部分核查，USB 实现来源尚未确定 |
| 本地检查点 | 用户反馈重新烧录成功后明确授权；完整输入/停止验收与来源核实仍待完成 |

生成镜像、组件、sdkconfig 和带本机路径的构建日志忽略；可提交证据只保留脱敏结果/源码 hash，不含私人路径、串口、截图或凭据。不承诺跨机器二进制一致。

## 基线整理

整理证据见 [preparation-check.json](../artifacts/baseline/preparation-check.json)。
随后用户提供 esptool 成功写入/哈希校验输出，并明确要求首次本地提交。
脱敏当前范围见 [checkpoint.json](../artifacts/baseline/checkpoint.json)，
上面的迁移记录仍为历史证据。候选 manifest 生成在被忽略的 `build/`；
按[基线验收步骤](baseline-acceptance.zh-CN.md)对准确镜像哈希反馈结果。
手机输入/停止验收与来源核实仍待完成，不宣称稳定基线或已可公开发行。
剩余门槛见[开源发布准备](release-preparation.zh-CN.md)。

## 后续真机验收，另行授权

用迁移镜像 hash 和合成笔记验证：以太网/HID 枚举和 60 秒 READY/PONG、完整竖屏预览、仅 observe、单个 tap/ASCII/drag 及视觉确认、输入中 Stop/按钮/拔线且无新动作/重放、无 Mac多步闭环。按需记录板型版本/系统/客户端/模型，保留失败和未知局部效果。若批准 E2E，可复现脱敏证据存 `artifacts/e2e/<scenario>/<run-id>/`。执行前 deviceVerified=false。

## MCP 开发候选

[build-check.json](../artifacts/mcp/build-check.json) 记录独立的
`feat/device-mcp` 当前候选，原 `0.1.0-mcp` 构建保留在
[initial-build-check.json](../artifacts/mcp/initial-build-check.json)。编译/打包和配置、原动作函数体对比、内置 JSON/
JavaScript、DER 结构及文档静态检查通过；`usb.c` 与 PROTOCOL.md 和本地检查点
字节一致，新增互斥/UI/身份代码单独实现。[设备 MCP](mcp.zh-CN.md)说明配置与运行
限制。未进行新烧录、手机、显示、TLS 信任、互通、延迟或持续运行验证；运行峰值堆内存
未测量，原来源及验收缺口保留。

随后用户反馈屏幕黑屏，但准确已烧录镜像/复位状态尚未确认。候选 `0.1.1-mcp`
增加扩展芯片唤醒/处理间隔/休眠配置及供电写入回读，提供仅 USB 的只读启动诊断。
其编译/静态结果与该反馈分开，不声称设备已验证修好，也未执行新的设备操作。

随后用户反馈屏幕点亮，手机配置流程出现 `USB origin/host required`，未提供准确镜像/
烧录记录。固定 IDF HTTP/lwIP 源码及当前 IPv6 配置解释了另一地址类型缺陷：双栈监听
把 IPv4 映射成 `AF_INET6`，旧检查只接受 `AF_INET`。候选 `0.1.2-mcp` 识别两种
表示，仍仅接受原准确 USB 地址，兼容 HTTP 默认端口等价写法，并返回固定拒绝类别。
静态/编译证据记录，手机配对、信任及 MCP 输入成功仍待验证。

候选 `0.2.0-mcp` 实现用户的触屏为主、设备内中英切换要求，保留 USB/ANAN1 原源码
契约，增加原生 CST820B、共用按钮布局/命中区、画面/手势授权检查、独立语言存储。
编译/打包、双语字形覆盖、22张静态文档预览、布局/文字边界通过，准确字体来源/
派生/OFL 声明保留。预览均为虚拟数据，不是手机/设备截图。触摸、实际停止延迟、
语言记忆及 MCP 手机验收未验证，未执行新的设备操作。

候选 `0.2.1-mcp` 按用户确认移除三分钟控制授权时限。授权不持久化，
停止/拔线/重启会撤销，其他撤销路径及会话/动作限制保留。编译/静态/布局
检查单独记录；尚未进行超过180秒的真机验证。

前一版触屏构建保留于
[touch-ui-build-check.json](../artifacts/mcp/touch-ui-build-check.json)，
[build-check.json](../artifacts/mcp/build-check.json) 现对应 `0.2.1-mcp`。

候选 `0.3.0-mcp` 增加 HTTP80 MCP 与物理确认后的同页配置导出，保留可选
HTTPS；HTTP 配置不再要求 iOS 证书信任。编译/打包、双语静态布局/字形及
JavaScript/源码/链接检查记于当前 MCP 构建记录，上一版持续授权构建保留于
[continuous-control-build-check.json](../artifacts/mcp/continuous-control-build-check.json)。
Agent 未执行新的烧录、证书删除、板上重置、HTTP 手机互通或剪贴板/真机运行，
这些仍需用户设备检查。

用户随后反馈 HTTP 新用户流程已验证且体验良好，见[脱敏反馈](../artifacts/mcp/http-user-feedback.json)。
这是用户反馈，没有独立核对镜像哈希或完整场景记录，不等于下一候选已验收。

候选 `0.4.0-mcp` 移除 ANAN1 TCP28766、解析/心跳/旧仲裁与来源切换 UI，
保留 USB/HID 输入序列，初始化 MCP 但不自动授权。旧 HTTP 构建保留于
[http-onboarding-build-check.json](../artifacts/mcp/http-onboarding-build-check.json)。
源码/符号/编译/打包/文档渲染检查与新烧录/真机运行分开，仅 MCP 镜像仍待验收。
