# Minimal board porting

English · [简体中文](porting.zh-CN.md)

Use [anan-iphone-use](../.agents/skills/anan-iphone-use/SKILL.md) to guide a user through hardware identification, the smallest port, flashing and phone setup. **Only M5Stack StopWatch is currently implemented.** Other boards and headless operation below are adaptation requirements, not existing support. Keep observation, models and the agent in the client.

## Confirm hardware before editing

Collect the exact board/chip/revision, official documentation and schematic, native USB D+/D− route and connector, flash/PSRAM, boot/download procedure, power routing/controller, usable buttons and active levels, and display/touch hardware or their absence. Confirm the phone and client too. Browse official chip/board and ESP-IDF documentation for the actual target; record confirmed facts and unknowns. A USB-UART or fixed USB Serial/JTAG connector does not by itself provide programmable USB Device HID/NCM. Do not flash the StopWatch image onto another board.

If the chip or board route cannot support this USB Device stack, explain the specific blocker and recommend a supported board or a separately agreed hardware design. Do not pretend that a pin change or compilation establishes compatibility.

Read `AGENTS.md`, [design](design.md), [source notices](../THIRD_PARTY.md) and the source. Inspect existing changes and propose the minimal affected files and physical interaction before implementing material architecture, power, dependency or protocol changes. Once scope is agreed, make the adaptation and compile it; do not stop at a generic porting plan. Preserve the StopWatch target and unrelated edits. Use a small explicit board variant or isolated worktree when useful, rather than a new generic board framework.

## Current coupling to inspect

| Area | Current StopWatch assumption | Porting responsibility |
| --- | --- | --- |
| `sdkconfig.defaults`, build configuration | ESP32-S3, 16MB flash and current memory settings | Select the actual chip and verified flash/PSRAM settings; unsupported chips need more than configuration |
| `partitions.csv`, `package.py`, `flash.sh` | Identity NVS at `0xf00000`; packaging/flashing hardcodes S3/16MB and the merged-image bound | Fit all partitions into actual flash; update variant packaging, image bound, manifest board identity and flash command together; avoid erasing identity during normal upgrades |
| `main/board.c` | I2C SDA47/SCL48, M5PM1 `0x6e`, register `0x07` bit5, active-low stop GPIO1/2 | Replace only the confirmed board's power/buttons; do not issue M5-specific power writes on another board or enable a supply that backfeeds the phone |
| `main/display.c`, `main/touch.c` | CO5300 466×466 QSPI, M5IOE1 and CST820B | Adapt confirmed screen/touch hardware, or implement the reviewed headless path below |
| `main/device.c`, `main/ui.c`, `main/ui_layout.c` | Visible-frame checks gate pairing and control | Retain StopWatch checks; provide a distinct physical authorization path for headless hardware |
| `main/usb.c`, `main/actions.c`, `main/mcp.c` | USB NCM/HID, input timing, six MCP tools and sessions | Reuse unchanged where possible; preserve release/stop, no replay and explicit fresh grants |

Treat tool/session behavior, HID descriptors, USB identifiers and phone subnet checks as stable. Do not restore retired ANAN1/TCP28766. Do not add Wi-Fi/BLE, screen capture or [Anan](https://anan.sh) code as incidental work. Record exact source versions, reused files, modifications and licenses before adding third-party code.

## Boards without a screen

Current `main/device.c` requires `screen_ok` and a current rendered frame for pairing and enabling control. Removing display initialization alone cannot make a headless board usable. Setting `screen_ok = true`, auto-confirming pairing or enabling at boot/reconnect bypasses the existing interaction and is not a port.

Agree a small physical workflow suited to the actual board. Prefer a usable **USER button plus an LED**, keeping configuration and JSON on the existing phone web page. A single-button design may use state-specific gestures for entering setup, confirming a pending pairing and separately enabling control; define durations and feedback explicitly before coding. Do not assume a BOOT/RESET button is safe to repurpose: check strap pins, reset wiring and boot behavior.

The design must provide all of these:

- **Enter setup deliberately:** open a bounded setup window with physical presence; do not leave enrollment open forever.
- **Confirm pairing deliberately:** use a fresh physical gesture tied to the pending setup. Without a device screen, the StopWatch six-digit visual comparison is unavailable; choose and explain a real alternative (for example LED code feedback or an explicitly agreed physical-confirmation flow). Do not tell the user that they compared a code visible only on the phone. Keep the pairing/config API and Bearer behavior unchanged where possible; discuss any required contract change first.
- **Enable separately:** importing JSON or completing pairing must not grant input. Require a new physical action after setup, provide understandable state feedback, and never persist or automatically restore the grant.
- **Stop immediately:** any assigned stop press during control/input stops first, consumes that gesture until release and cannot also re-enable or reset. Keep this path independent of display, I2C and web-page availability; retain MCP `stop` and USB-detach revocation.
- **Show state and recover:** make setup, ready, control and failure distinguishable with the actual LED/phone feedback. Define a separate intentional pairing-reset gesture; preserve language/identity storage rules where applicable. If there is no usable physical input, propose an external button or another reviewed interaction instead of silently removing authorization.

Implement headless authorization as a narrow board-specific path, reusing existing pairing, grant, cancellation and stop primitives. Keep current StopWatch display-failure blocking intact. A broken screen on a StopWatch must not silently turn it into a headless device. Update English/Chinese instructions with the actual gestures and feedback after implementing a variant; do not apply the StopWatch tap sequence to it.

## Build, flash and finish setup

Before coding, define the board's acceptance scenarios: power and USB enumeration, setup/confirmation, independent enable, physical/MCP/detach stop, and no automatic replay or restored grant. Build/package for the actual target with matching offsets and flash size, and record source/image hashes and tool versions. Static success does not establish USB enumeration, button latency or phone input.

Provide a command using the user's current serial port and variant image. Agent-executed flashing, hardware probes/inputs and E2E require current target/scope authorization; an explicit current flash request can supply it. Never run the StopWatch `flash.sh` unchanged for a different chip/flash layout, erase the whole chip by default or modify eFuses.

After verified writing, follow the skill's phone sequence: connect iPhone ↔ native USB, configure USB Ethernet `192.168.7.2/24` without gateway/DNS, enable AssistiveTouch, install/open the phone client, physically pair through `http://192.168.7.1`, copy/import MCP JSON, and physically enable using the variant's actual controls. HTTP does not require a certificate. The client owns observation and task permission; desktop MCP support does not establish access to the phone USB subnet.

Report compiled, flashed, user-reported and device-verified results separately. Retain unresolved provenance and current StopWatch evidence. No automatic commit or publication.
