# Anan iPhone Use 文档导航

[English](README.md) · 简体中文

| 主题 | 文档 |
| --- | --- |
| 项目用途与源码组织 | [项目](../README.zh-CN.md)、[设计](design.zh-CN.md)、[固件模块](../main/README.zh-CN.md) |
| 构建、烧录与手机配置 | [配置与恢复](setup.zh-CN.md) |
| Agent 引导安装与板型适配 | [anan-iphone-use skill](../.agents/skills/anan-iphone-use/SKILL.zh-CN.md)、[最小化板型适配](porting.zh-CN.md) |
| 客户端与线协议 | [客户端接入](client.zh-CN.md)、[历史 ANAN1](../PROTOCOL.md) |
| 证据与首次提交 | [验证记录](validation.zh-CN.md)、[基线验收](baseline-acceptance.zh-CN.md) |
| 许可与来源处理 | [GPL决定与保留声明](license-plan.zh-CN.md) |
| 发布与贡献 | [开源发布准备](release-preparation.zh-CN.md)、[来源声明](../THIRD_PARTY.zh-CN.md)、[贡献说明](../CONTRIBUTING.zh-CN.md) |
| 后续变更 | [路线图](roadmap.zh-CN.md)、[MCP 开发决策](mcp-development.zh-CN.md) |

[PROTOCOL.md](../PROTOCOL.md) 是未改写的历史 ANAN1 协议/状态契约；当前固件仅运行 MCP，不提供28766服务。验证记录区分源码、编译、用户反馈与设备执行；MCP/中英触屏 UI 已有开发实现，编译/静态预览及用户反馈与具体验收分开记录，见[配置与端点](mcp.zh-CN.md)。

[触屏 UI 与语言切换](touch-ui.zh-CN.md)：原生 CST820B、圆屏布局、字体来源及预定义验证。
