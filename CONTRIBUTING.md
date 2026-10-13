# Contributing

English · [简体中文](CONTRIBUTING.zh-CN.md)

Anan iPhone Use (`anan-iphone-use`) maintains device-side iPhone automation firmware for M5Stack StopWatch. Start with the
[design](docs/design.md), [roadmap](docs/roadmap.md), [validation](docs/validation.md)
and [source notices](THIRD_PARTY.md). The repository is hosted privately at
[ananhq/anan-iphone-use](https://github.com/ananhq/anan-iphone-use); public release remains pending.

## Scope

Agree the problem and acceptance scope with the maintainer before implementing
an external contribution. Discuss architecture, protocol, dependencies, board
support and license changes first. Keep phone capture, agents, model connections
and [Anan](https://anan.sh)-specific client code in their owning repositories.

MCP is the sole current control protocol; see [endpoint and lifecycle](docs/mcp.md).
[PROTOCOL.md](PROTOCOL.md) is the retained historical ANAN1 contract, not a
service in the current build. Preserve
USB descriptors, input timing, power and stop behavior unless a change is
explicitly agreed with the client. Do not automatically reconnect or replay input.
MCP and Chinese/English touch UI are implemented development code with pending device acceptance; Wi-Fi/BLE remain planned. See the [roadmap](docs/roadmap.md) and [GPL license and source handling](docs/license-plan.md).

## Changes and validation

- Keep changes small and record exact origins and applicable notices for reused
  code. An SPDX header alone does not resolve an unknown implementation origin.
- Update English and Simplified Chinese documentation together. State what was
  compiled, observed on hardware, reported by a user, or remains unverified.
- Use ESP-IDF v5.5.5 and the checked-in dependency lock. Run `./build.sh` for
  firmware changes, shell syntax checks for shell changes and Python syntax
  checks for Python changes. Documentation changes need link/format checks.
- Define device scenarios before implementation. Obtain approval for the current
  scope before flashing, HID input, live model calls or E2E. Do not write unit
  tests after implementation.
- Keep generated builds, managed components, local `sdkconfig`, caches, raw logs,
  credentials, pairing records, private screens and device identifiers out of
  commits. Share only sanitized, relevant evidence.

Describe the concrete behavior changed, source handling, checks performed and
remaining runtime gaps in the change description. A passing build is not device
acceptance. See the [baseline procedure](docs/baseline-acceptance.md) and
[release preparation](docs/release-preparation.md) for the current gates.
