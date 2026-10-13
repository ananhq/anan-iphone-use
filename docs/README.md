# Anan iPhone Use documentation

English · [简体中文](README.zh-CN.md)

| Topic | Document |
| --- | --- |
| Purpose and source layout | [Project](../README.md), [design](design.md), [firmware module](../main/README.md) |
| Build, flash and phone configuration | [Setup and recovery](setup.md) |
| Agent-guided installation and board adaptation | [anan-iphone-use skill](../.agents/skills/anan-iphone-use/SKILL.md), [minimal board porting](porting.md) |
| Client and wire contract | [Client integration](client.md), [Historical ANAN1](../PROTOCOL.md) |
| Evidence and first commit | [Validation](validation.md), [baseline acceptance](baseline-acceptance.md) |
| License and source handling | [GPL decision and retained notices](license-plan.md) |
| Publication and contribution | [Release preparation](release-preparation.md), [source notices](../THIRD_PARTY.md), [contributing](../CONTRIBUTING.md) |
| Future changes | [Roadmap](roadmap.md), [MCP development decisions](mcp-development.md) |

[PROTOCOL.md](../PROTOCOL.md) is the unchanged historical ANAN1 protocol/state definition; current firmware runs only MCP and no port28766 service. Validation records distinguish source, compilation, user reports and device execution. MCP and Chinese/English touch-first UI are implemented development code; compilation/static previews and user reports are separate from scoped device acceptance. See [MCP setup and endpoint](mcp.md).

[Touch UI and language](touch-ui.md): native CST820B, round-screen layout, font provenance and predefined validation.
