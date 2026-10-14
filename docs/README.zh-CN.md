# Anan iPhone Use 文档导航

[English](README.md) · 简体中文

## 使用与接入

| 主题 | 文档 |
| --- | --- |
| 项目概览 | [项目](../README.zh-CN.md) |
| 构建、烧录与手机配置 | [配置与恢复](setup.zh-CN.md)、[MCP 配对与端点](mcp.zh-CN.md) |
| 客户端接入 | [MCP 参数与会话](client.zh-CN.md) |
| Agent 引导安装与板型适配 | [anan-iphone-use skill](../.agents/skills/anan-iphone-use/SKILL.md)、[最小化板型适配](porting.zh-CN.md) |

## 开发

| 主题 | 文档 |
| --- | --- |
| 文件、工具与本地产物 | [仓库组织](repository.zh-CN.md)、[固件模块](../main/README.zh-CN.md) |
| 架构与范围 | [设计](design.zh-CN.md)、[路线图](roadmap.zh-CN.md) |
| 实现决策 | [MCP 开发](mcp-development.zh-CN.md)、[触屏 UI 与语言](touch-ui.zh-CN.md) |
| 扩展 HID 工具与验收 | [工具参考](hid-tools.zh-CN.md)、[预定义范围](hid-development.zh-CN.md) |
| 贡献流程 | [贡献说明](../CONTRIBUTING.zh-CN.md) |

## 证据与分发

| 主题 | 文档 |
| --- | --- |
| 已记录检查与待完成验收 | [验证记录](validation.zh-CN.md)、[基线验收](baseline-acceptance.zh-CN.md)、[证据目录](repository.zh-CN.md#验证记录) |
| 许可与来源处理 | [GPL 决策](license-plan.zh-CN.md)、[来源声明](../THIRD_PARTY.zh-CN.md) |
| 发布门槛 | [开源发布准备](release-preparation.zh-CN.md) |
| 已退役协议 | [历史 ANAN1 契约](../PROTOCOL.md) |

[PROTOCOL.md](../PROTOCOL.md) 是未改写的历史 ANAN1 协议/状态契约；当前固件仅运行 MCP，不提供28766服务。验证记录区分源码、编译、用户反馈与设备执行；MCP/中英触屏 UI 已有开发实现，编译/静态预览及用户反馈与具体验收分开记录，见[配置与端点](mcp.zh-CN.md)。
