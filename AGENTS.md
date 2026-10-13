# Repository instructions

## Purpose and scope

- This repository provides ESP32 firmware for operating an iPhone through USB input and MCP. M5Stack StopWatch is the current default implementation, intended for StopWatch users. Other ESP32 boards, including boards without screens, are welcome through hardware-specific adaptation; they are not all supported out of the box.
- [Anan](https://anan.sh) is the first integrated client. Other MCP clients and agents are welcome when they provide screen observation, task authorization and a network path to the device. Firmware owns input execution and device configuration; clients own observation, reasoning, planning and models. Do not copy or modify client/agent repositories as incidental firmware work.
- For installation, board adaptation or phone/client setup, follow [.agents/skills/anan-iphone-use/SKILL.md](.agents/skills/anan-iphone-use/SKILL.md). Guide the user from their actual board through firmware, phone USB networking, pairing, configuration import and control. The skill guides client integration; it does not implement an agent or screen capture.

## Development and board adaptation

- Read [docs/roadmap.md](docs/roadmap.md), [docs/validation.md](docs/validation.md) and [THIRD_PARTY.md](THIRD_PARTY.md) before work. Inspect Git status and preserve existing changes. Migration, baseline acceptance, MCP, display UI and board-specific acceptance are separate scopes.
- For another board, read [docs/porting.md](docs/porting.md). Confirm the actual chip, native USB Device capability/routing, flash, power, controls and display/touch hardware. Do not assume every ESP32 or USB connector supports HID/NCM. Never flash a StopWatch image onto another board.
- Use the smallest confirmed board-specific design, preserve the StopWatch target, and avoid an unsolicited generic board framework. Update the variant's configuration, partitions, packaging/manifest and flash command together when required. Once the adaptation scope is agreed, implement and compile it rather than stopping at a generic plan.
- Discuss material architecture, protocol, power, dependency and license changes before implementation. Headless boards need a deliberate physical pairing, separate enable and immediate stop workflow; do not bypass authorization by faking display readiness or auto-confirming. A broken StopWatch screen must not silently become headless operation.
- `main/` and build/configuration files own firmware. MCP is the current sole control protocol. [PROTOCOL.md](PROTOCOL.md) is the sole authoritative historical ANAN1 contract; preserve it and do not restore the retired TCP28766 service or define a second translated contract.
- Preserve existing HID descriptors, input timing, StopWatch power and stop behavior unless a change is explicitly agreed. Adapt another board's power/controls to confirmed hardware rather than applying StopWatch-specific register writes. No implicit input retry, replay, reconnection or restored control grant.

## Source handling and documentation

- Before copying code, record exact source versions, file origins, modifications and applicable notices; unresolved provenance must remain explicit. Preserve GPL-3.0-only and applicable third-party terms. Do not change licenses without confirmation.
- Document first-party modules and board-specific setup in English and Simplified Chinese, updating both together. Use the actual board's controls and feedback; do not give StopWatch touchscreen instructions to a headless user. Mark planned capabilities and unverified behavior honestly.
- Never include model credentials, pairing records, device identifiers, private screens, machine paths or unfiltered logs in committed artifacts. Keep `build/`, `managed_components/`, generated `sdkconfig`, caches and local exports ignored. Preserve tracked defaults, dependency locks and sanitized historical evidence.

## Validation and authorization

- Compilation/static/document checks are allowed. Agent-executed flashing, device probes/actions, live providers, simulators and E2E need explicit authorization for the current identified target and scope. A current explicit flash request supplies flashing authorization once the target is identified; do not ask again. Invoking the skill alone or earlier device permission does not authorize device execution. Giving the user a command does not execute it.
- Define device acceptance scenarios before implementation. Distinguish compilation, packaging, flashing, user reports and device verification; success at one level does not establish another or support for a different board. Do not write unit tests after implementation.
- Commit only when requested. Form an agreed baseline commit only after its acceptance scope and source handling are confirmed. No push, GitHub repository creation or Release without publication authorization.
