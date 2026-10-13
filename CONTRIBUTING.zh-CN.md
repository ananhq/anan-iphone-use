# 贡献说明

[English](CONTRIBUTING.md) · 简体中文

Anan iPhone Use（`anan-iphone-use`）维护面向 M5Stack StopWatch 的设备端 iPhone 自动化固件。开始前阅读[设计](docs/design.zh-CN.md)、
[路线图](docs/roadmap.zh-CN.md)、[验证记录](docs/validation.zh-CN.md)和
[来源声明](THIRD_PARTY.zh-CN.md)。仓库位于私有的 [ananhq/anan-iphone-use](https://github.com/ananhq/anan-iphone-use)，公开发布仍待完成。

## 范围

外部贡献先与维护者约定问题和验收范围。架构、协议、依赖、板型和许可证变更先讨论。
手机采屏、Agent、模型连接和 [Anan](https://anan.sh) 专用客户端代码保留在各自所属仓库。

MCP 是当前唯一控制协议，见[端点与生命周期](docs/mcp.zh-CN.md)。
[PROTOCOL.md](PROTOCOL.md) 保留历史 ANAN1 契约，当前构建不运行该服务。除非已与客户端明确约定变更，
保留 USB 描述、输入时序、供电与停止行为。不自动重连或重放输入。
MCP 和中英触屏 UI 已有开发实现，真机验收仍待完成；Wi-Fi/BLE 仍为规划。见[路线图](docs/roadmap.zh-CN.md)与[GPL许可与来源处理](docs/license-plan.zh-CN.md)。

## 修改与验证

- 保持修改范围小，复用代码时记录准确来源和适用声明。单独的 SPDX 头不能解决未知实现来源。
- 同步更新英文和简体中文文档，区分编译、真机观察、用户反馈与尚未验证的行为。
- 使用 ESP-IDF v5.5.5 和仓库内锁文件。固件修改运行 `./build.sh`；Shell 和 Python
  修改分别检查语法，文档修改检查链接和格式。
- 实现前定义真机场景。烧录、HID 输入、真实模型调用和 E2E 需要当前范围的授权。
  不在实现后补写单元测试。
- 构建目录、托管组件、本地 `sdkconfig`、缓存、原始日志、凭据、配对记录、私人屏幕和
  设备标识不进入提交；只分享脱敏且与结论相关的证据。

变更说明写清行为、来源处理、已做检查和运行缺口。编译通过不等于设备验收。
当前门槛见[基线验收步骤](docs/baseline-acceptance.zh-CN.md)和
[开源发布准备](docs/release-preparation.zh-CN.md)。
