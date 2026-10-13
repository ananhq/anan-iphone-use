---
name: anan-iphone-use
description: Set up this repository's ESP32 iPhone MCP firmware. Use when a user wants to flash M5Stack StopWatch, minimally adapt another ESP32 board (including boards without screens), or complete iPhone USB network, pairing and MCP client setup. Does not implement the client agent or screen capture.
license: GPL-3.0-only
---

# Anan iPhone Use

Guide the user from their actual board to a connected iPhone MCP client. Use the user's language, small steps and concrete commands. Reuse the StopWatch implementation; port other boards with the smallest confirmed changes. Firmware executes USB input; the client owns observation, reasoning, planning and task permission.

## Establish the starting point

- Locate the repository relative to this file: `../../..`. Read its `AGENTS.md`, [roadmap](../../../docs/roadmap.md), [validation](../../../docs/validation.md) and [source notices](../../../THIRD_PARTY.md). Inspect Git status and preserve existing changes. Do not require a clean tree or overwrite another session's work.
- Ask only for missing essentials: exact board/chip/revision, computer OS, USB-C iPhone and chosen phone MCP client, and whether firmware is already flashed. Distinguish the computer agent helping with setup from the phone client operating the iPhone.
- For M5Stack StopWatch, follow the reuse path below. For any other board, read the [porting guide](../../../docs/porting.md), including its headless branch, before changing code. Never flash the StopWatch image onto another board or assume every ESP32 has suitable native USB.
- Compilation/static/document checks may proceed. Agent-executed flashing, device probes/actions, live providers and E2E need explicit authorization for the current identified target and scope. A request to use this skill alone is not device permission; a current explicit flash request is sufficient once the target is identified, without asking again. Offering a command for the user to run does not execute it.
- Keep Anan and other client repositories untouched. Do not commit, publish, change remotes or rename folders unless requested. Preserve GPL and upstream notices; record any newly reused code accurately.

## Reuse StopWatch and flash

Read [setup and recovery](../../../docs/setup.md) and inspect `build.sh`, `flash.sh`, `package.py`, `sdkconfig.defaults` and `partitions.csv` before issuing commands.

1. Reuse a local image only when its manifest identifies this board and matches the image SHA-256, current source hashes, dependency lock and build settings. A compilation flag is not device validation. A fresh clone has no published Release binary; build locally unless a verified release is actually available.
2. Discover the installed ESP-IDF environment. If absent, guide installation of official **v5.5.5** with submodules, `install.sh esp32s3` and sourcing `export.sh` (use the official Windows equivalents where needed). Do not hardcode the maintainer's temporary Python/IDF paths or silently change SDK/dependency versions. In the sourced environment run `./build.sh`; on Windows use the equivalent IDF build plus `python package.py build`. Inspect the generated manifest and hash before flashing.
3. Have the user connect StopWatch's native Type-C port to the computer with a data cable. Use official download-mode instructions: hold power about two seconds until the green LED lights, then release. Discover the current serial port from OS enumeration/connection changes; never reuse a port number from history. For a busy port, identify its owner and close the known monitor; do not kill unrelated processes.
4. Give one ready-to-run command using the actual discovered port and available Python environment. Normal path: `./flash.sh <actual-port>`. If already in download mode and normal reset cannot connect, use the manual `--before no_reset` command in the setup guide. StopWatch uses chip `esp32s3`, flash `16MB`, merged image `build/anan-stopwatch-flash.bin` at **0x0**. Never use the application offset for a merged image.
5. Require completed write and hash verification; `Connecting` or a running stub is insufficient. Short-press power to reset/exit download mode. If failure occurs, diagnose target/port/mode/image before retrying. Do not default to full-chip erase, factory recovery, eFuse changes or Secure Boot. Normal upgrades preserve pairing NVS; reset/repartitioning may not.

## After flashing: connect and configure the phone first

Read [phone setup](../../../docs/setup.md) and [MCP setup](../../../docs/mcp.md). Start here if the correct firmware is already installed. For a ported board, substitute its approved physical steps; do not tell a headless user to tap a screen.

1. **Move the cable from the computer to the unlocked iPhone.** Connect iPhone ↔ device using the confirmed native USB/data connector. On iPhone, open the newly appearing USB Ethernet interface in Settings and set manual IPv4 **192.168.7.2**, mask **255.255.255.0**, router empty and no DNS. The board is **192.168.7.1**. This is a USB local network, not Wi-Fi setup or an internet gateway; retain Wi-Fi/cellular for the client's model connection. If the interface does not appear, resolve cable/reset/USB enumeration before pairing.
2. Enable Settings → Accessibility → Touch → AssistiveTouch. Use portrait orientation and US hardware keyboard layout. This firmware types ASCII, not Chinese/Unicode.
3. Guide installation/opening of the chosen **phone client** through its currently verified official distribution route. If no route is known, ask for it instead of inventing an App Store/TestFlight link or building/modifying the client. Allow local-network permission. [Anan](https://anan.sh) supports HTTP MCP; other clients must support the transport, Bearer headers and JSON import/configuration. Verify their actual UI/documentation rather than inventing menu names.
4. On StopWatch tap **Settings → Connect MCP app**. In phone Safari open **http://192.168.7.1**, tap Start pairing, compare the six-digit code and confirm on StopWatch. The setup window lasts ten minutes. HTTP needs **no downloaded certificate, profile or trust settings**. HTTPS is optional, not the default onboarding path.
5. After confirmation the same page generates MCP JSON. Have the user copy it or download the JSON and import into their phone client. It includes **http://192.168.7.1/mcp** and the device's Bearer header. Keep the token/configuration on the user's phone; do not ask them to paste it into agent chat or commit it. For HTTP clipboard failure, select/copy manually or use the download fallback. Do not handwrite or guess a token.
6. Tap **Imported, go back → Start control** on StopWatch. For a ported/headless board, use its separately implemented fresh physical enable. Enable the client's task control and screen sharing separately; neither pairing nor MCP discovery grants hardware input. StopWatch touch, either front button, `stop` or unplugging revokes control; headless users need their board-specific physical stop instruction.
7. When authorized for connection checks, confirm MCP discovery and `device_status` without sending HID. The six device tools are `device_status`, `tap`, `drag`, `key`, `type_ascii`, `stop`. Observation is client-provided (e.g. Anan's `phone.observe`), not a seventh device tool. Anan system screen sharing currently requires iOS 27+; verify the actual client version and OS.

## Finish and diagnose

- A computer/cloud agent cannot directly reach the attached phone's USB subnet merely because it supports MCP. Explain this before importing phone-local JSON on a desktop. No network bridge, Wi-Fi or BLE is supplied by this skill or current firmware.
- If the user authorizes input validation, start with observation, one small action on a synthetic test note and another observation. HID/tool completion does not prove the app outcome. Test the approved stop path; never automatically replay lost/partial inputs or re-enable after reconnect.
- For `USB origin/host required` or timeouts, check the phone USB IP/mask, phone-origin access, HTTP URL, local-network permission and imported header against current source/docs. Do not revive retired TCP port28766 or broadly disable address checks. Use sanitized `/status` information for a blank StopWatch screen; do not bypass display authorization to make it work.
- Summarize the actual board, source changes if any, image hash/build version, flash result, phone network and MCP status, plus the user's next operation. Distinguish compiled, flashed, user-reported and device-verified outcomes. Leave unexecuted checks pending; keep identifiers, tokens and private screens out of repository artifacts.
