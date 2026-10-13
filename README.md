# Anan iPhone Use

English · [简体中文](README.zh-CN.md)

**An MCP server running on ESP32-S3 that operates a real iPhone through USB mouse and keyboard input.**

Connect it to a client agent to add **iPhone Use**: the client observes the screen, decides what to do, calls MCP tools, and checks the result. The firmware executes taps, drags, key presses and ASCII input. It runs no model and provides no phone screenshots.

**Anan is the first client. The current hardware target is M5Stack StopWatch.** Other agents can connect through MCP and provide their own screen observation.

## Demos and setup

| Using Notes | Using Xiaohongshu |
| --- | --- |
| Video link coming later | Video link coming later |

<!-- Replace the video placeholders above with the supplied demo links. -->

**1 · Connect and configure the local network:** connect the iPhone to StopWatch and set up USB Ethernet on the phone: manual IP `192.168.7.2`, mask `255.255.255.0`, no router or DNS, and enable AssistiveTouch. Then pair, import the MCP configuration and enable control as shown below.

<table>
<tr>
<th>2 · Open connection settings</th>
<th>3 · Match and confirm pairing</th>
<th>4 · Copy JSON on phone and import</th>
<th>5 · Go back and enable control</th>
</tr>
<tr>
<td><img src="docs/images/setup/en-settings.png" width="200" alt="StopWatch settings: Connect MCP app"></td>
<td><img src="docs/images/setup/en-pair-confirm.png" width="200" alt="StopWatch pairing confirmation: match the code on the phone"></td>
<td><img src="docs/images/setup/en-pair-ready.png" width="200" alt="StopWatch pairing ready: copy config on phone and return after import"></td>
<td><img src="docs/images/setup/en-home-active.png" width="200" alt="StopWatch home: control enabled, stop available"></td>
</tr>
</table>

*UI illustrations [rendered from the current firmware layout](tools/render_ui.py), not device screenshots; `123456` is a sample pairing code. Open `http://192.168.7.1` in phone Safari to pair and export configuration, with no certificate required. The skill below guides the full setup.*

## How it works

```mermaid
flowchart LR
    Phone["iPhone"] -->|"Screen sharing"| Agent["Client agent, such as Anan<br/>Observe · Reason · Plan"]
    Agent -->|"MCP actions"| Device["M5Stack StopWatch<br/>ESP32-S3 MCP server"]
    Device -->|"USB mouse / keyboard"| Phone
```

After each action, the client should observe again to verify its effect. Tool completion means input was sent, not that the intended operation succeeded.

## What you need

| Item | Requirement |
| --- | --- |
| Device | M5Stack StopWatch (ESP32-S3) and a USB-C data cable |
| Phone | USB-C iPhone with AssistiveTouch enabled |
| Client | Anan, or an HTTP MCP client that can reach the device's USB subnet |
| Screen observation | Supplied by the client; Anan's system screen sharing requires iOS 27+ |

## Quick start

```sh
git clone https://github.com/ananhq/anan-iphone-use.git
cd anan-iphone-use
```

*This is the planned publication address; cloning is available after the repository is public. You can open an existing local checkout now.*

Open your agent in the repository directory and enter:

```text
Use $anan-iphone-use to identify my board, flash the firmware and set up iPhone MCP.
If the skill is not discovered, first read .agents/skills/anan-iphone-use/SKILL.md.
```

The [anan-iphone-use skill](.agents/skills/anan-iphone-use/SKILL.md) reuses StopWatch firmware or minimally adapts another ESP32 for its actual hardware, including controls for boards without screens.

## MCP interface

Default endpoint: `http://192.168.7.1/mcp`. The exported pairing configuration includes the required Bearer header. An optional HTTPS endpoint is also retained.

| Tool | Function |
| --- | --- |
| `device_status` | Read connection and control status |
| `tap` | Tap a position |
| `drag` | Drag or swipe |
| `key` | Send a keyboard key |
| `type_ascii` | Type ASCII text |
| `stop` | Stop input and revoke control permission |

Screen observation belongs to the client and is not a device MCP tool. A desktop or cloud agent needs an explicit network path to the phone's USB subnet; MCP support alone does not make the device reachable. See [client integration](docs/client.md) for parameters and session behavior.

## Current limitations

- M5Stack StopWatch over wired USB is the current target. Wi-Fi/BLE is not implemented.
- Portrait, one pointer and US-layout ASCII input only; no direct Chinese text input.
- Other client interoperability, device regression of the latest MCP-only firmware and endurance remain to be verified. See [validation](docs/validation.md).

## License

The combined project is licensed under **[GPL-3.0-only](LICENSE)**. Binary distribution must include Corresponding Source and required build materials through a compliant GPL distribution method.

Upstream copyright notices and applicable licenses for third-party code and fonts are retained in [NOTICE](NOTICE) and [THIRD_PARTY](THIRD_PARTY.md).

## Comparison and credits

Thanks to **actell** for the USB NCM/HID source reference, and **esp32-iphone-agent** for exploring iPhone hardware automation.

| Project | Observation and agent | Action interface | iPhone input |
| --- | --- | --- | --- |
| Anan iPhone Use + Anan | Phone-side screen sharing and agent | MCP | USB absolute mouse and keyboard |
| [actell](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904) | Phone-side agent, CoreDevice display stream | CoreDevice | CoreDevice taps and drags |
| [esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18) | Computer-side AirPlay and Python agent | Wi-Fi TCP | USB relative mouse and keyboard |

actell also retains the USB NCM/HID route referenced here; the table describes its CoreDevice route. esp32-iphone-agent's MIT license and Copyright (c) 2026 Henry Yuan are preserved in the [complete notice](licenses/esp32-iphone-agent-MIT.txt). Exact versions, file origins and modifications are recorded in [THIRD_PARTY](THIRD_PARTY.md).
