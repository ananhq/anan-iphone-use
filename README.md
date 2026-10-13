# StopWatch firmware

English · [简体中文](README.zh-CN.md)

USB Ethernet and HID input firmware for **M5Stack StopWatch (ESP32-S3, 16MB Flash)**. A phone-side agent observes the screen and sends actions over one USB-C data cable. The device provides an absolute mouse and keyboard. Anan is the first client; the firmware itself runs no model and captures no phone screen.

This repository was extracted from Anan and is being prepared for open-source
publication. Its name/address remain provisional; no public repository or Release
is available. This local checkpoint records compilation/static checks and a
user-reported successful reflash. Phone input/stop acceptance and source-origin
resolution remain pending. The current scope is USB
control on StopWatch. Wi-Fi, BLE, MCP and display/touch UI are future work.

```text
phone capture → model → phone client → USB Ethernet / ANAN1
                                                ↓
                                      StopWatch → USB HID → phone
```

## Build and use

Install ESP-IDF **v5.5.5**, then source its `export.sh`:

```sh
./build.sh
shasum -a 256 build/anan-stopwatch-flash.bin
```

Build/package never accesses a serial port. The merged image and its source/hash
manifest are generated under ignored `build/`. Follow [setup and recovery](docs/setup.md)
for explicit-port flashing and phone configuration. Flashing replaces the factory
firmware; a blank device screen is expected. Runtime uses the phone's existing
model connection, with no Mac, Wi-Fi or BLE required by the board. Anan's current
capture client requires iOS 27+.

The user supplied a successful write/hash-verification report for the candidate
and authorized this first local checkpoint. Phone behavior still needs scoped
[baseline acceptance](docs/baseline-acceptance.md); historical input reports do
not substitute for that verification.

## Repository layout

| Path | Purpose |
| --- | --- |
| `main/` | StopWatch power/buttons, USB network/HID and command execution |
| `build.sh`, `package.py`, `flash.sh` | Pinned-IDF build, image packaging and explicit device writing |
| `sdkconfig.defaults`, `dependencies.lock` | Checked-in configuration and exact dependency resolution |
| `docs/` | Bilingual design, setup, integration, acceptance and roadmap |
| `artifacts/` | Sanitized migration and baseline evidence; no generated binaries |
| `licenses/`, `THIRD_PARTY.md`, `NOTICE` | Dependency notices and implementation-origin findings |

## Documentation

- [Background and ownership](docs/design.md), [client integration](docs/client.md).
- [ANAN1 protocol](PROTOCOL.md): the single authoritative wire/lifecycle contract; no HTTP/MCP today.
- [Porting guide and prompt](docs/porting.md), [validation](docs/validation.md), [roadmap and handoff](docs/roadmap.md).
- [Third-party provenance](THIRD_PARTY.md), [LICENSE](LICENSE), [NOTICE](NOTICE), [source snapshot](artifacts/migration/source-snapshot.json).
- [Baseline acceptance](docs/baseline-acceptance.md), [release preparation](docs/release-preparation.md), [contributing](CONTRIBUTING.md), [full documentation index](docs/README.md).

The first slice supports portrait, one pointer, US-layout ASCII and manual IPv4.
Either front button stops input. `DONE` is a HID report result; observe the phone
to establish whether the intended action succeeded. No automatic input replay.

Source provenance is partially unresolved, including the USB implementation.
Existing Apache-2.0 declarations are preserved; they do not establish independent
origin for every file. Resolve the [recorded source gaps](THIRD_PARTY.md) before
public distribution. Device verification and source handling are separate gates.

## Comparison and credits

Thanks to [ja1chen/actell](https://github.com/ja1chen/actell) and
[onehumanbeing/esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent)
for exploring phone agents and external input devices. The table describes this
repository with its Anan client and the upstream versions linked below.

| Project | Screen capture | Agent runs on | Command path | iPhone input |
| --- | --- | --- | --- | --- |
| This firmware + Anan | Phone-side ScreenCaptureKit | iPhone | USB Ethernet / ANAN1 TCP → StopWatch | USB absolute mouse and keyboard |
| [actell](https://github.com/ja1chen/actell/tree/e701786d1686823f9ab60524e689e387b7c3b904) | CoreDevice display stream | iPhone | VPN loopback, or ESP32-S3 USB NCM forwarding → CoreDevice | CoreDevice taps and drags |
| [esp32-iphone-agent](https://github.com/onehumanbeing/esp32-iphone-agent/tree/68584cfc8b3f227d6324ec7c8008c48b53f20a18) | AirPlay → computer-side UxPlay | Computer / Python | Wi-Fi TCP → ESP32-S3 | USB relative mouse and keyboard |

Actell also retains an older ScreenCaptureKit + USB NCM/HID route, which is the
reference relevant to this firmware; the table describes its later CoreDevice
route. esp32-iphone-agent informed the external USB HID/agent architecture.
Exact reference versions, implementation-origin gaps and applicable notices
are recorded in [THIRD_PARTY.md](THIRD_PARTY.md).
