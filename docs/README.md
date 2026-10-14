# Anan iPhone Use documentation

English · [简体中文](README.zh-CN.md)

## Use and integration

| Topic | Document |
| --- | --- |
| Project overview | [Project](../README.md) |
| Build, flash and phone configuration | [Setup and recovery](setup.md), [bond-preserving updates](flashing.md), [MCP pairing and endpoint](mcp.md) |
| Wi-Fi MCP and experimental BLE input | [Wi-Fi MCP and experimental BLE input](wireless.md) |
| Client integration | [MCP parameters and sessions](client.md) |
| Agent-guided installation and board adaptation | [anan-iphone-use skill](../.agents/skills/anan-iphone-use/SKILL.md), [minimal board porting](porting.md) |

## Development

| Topic | Document |
| --- | --- |
| Files, tools and local outputs | [Repository layout](repository.md), [firmware modules](../main/README.md) |
| Architecture and scope | [Design](design.md), [roadmap](roadmap.md) |
| Implementation decisions | [MCP development](mcp-development.md), [touch UI and language](touch-ui.md) |
| Extended HID tools and acceptance | [Tool reference](hid-tools.md), [predefined scope](hid-development.md) |
| Contribution process | [Contributing](../CONTRIBUTING.md) |

## Evidence and distribution

| Topic | Document |
| --- | --- |
| Recorded checks and pending acceptance | [Validation](validation.md), [baseline acceptance](baseline-acceptance.md), [evidence directories](repository.md#validation-records) |
| License and source handling | [GPL decision](license-plan.md), [source notices](../THIRD_PARTY.md) |
| Publication gates | [Release preparation](release-preparation.md) |
| Retired protocol | [Historical ANAN1 contract](../PROTOCOL.md) |

[PROTOCOL.md](../PROTOCOL.md) is the unchanged historical ANAN1 protocol/state definition; current firmware runs only MCP and no port28766 service. Validation records distinguish source, compilation, user reports and device execution. MCP and Chinese/English touch-first UI are implemented development code; compilation/static previews and user reports are separate from scoped device acceptance. See [MCP setup and endpoint](mcp.md).

[Wireless setup acceptance](wireless-setup-development.md) defines the fixed-hostname, network-memory and temporary-hotspot slice.

[Tool discovery guidance](tool-discovery.md) describes task keywords, optional whole-group browsing and unchanged schema/authorization boundaries.
