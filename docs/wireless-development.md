# Wireless development and acceptance

English · [简体中文](wireless-development.zh-CN.md)

Approved scope: keep the existing USB NCM/HID path; add local Wi-Fi HTTP MCP and
an experimental BLE HID backend for M5Stack StopWatch. Clients still supply
observation. No client code, AirPlay receiver, cloud relay, automatic input
replay, restored control grant, or additional advertised MCP tools.

Before implementation, define these acceptance scenarios:

- Offline: retain USB descriptors and sender bodies (backend symbol renaming
  only), existing action timing and seven tool schemas; exercise explicit
  backend selection with simulated USB/BLE connectivity and no fallback.
- Offline: verify BLE report lengths, references, coordinate endpoints and
  separate absolute-position/relative-button encoding; encrypted and subscribed
  reports only, bounded completion waits, no report retries.
- Static/build: compile with the pinned IDF; review same-origin/interface checks,
  USB-only initial Wi-Fi configuration, token-protected LAN MCP, configuration
  persistence, StopWatch controls, and bilingual UI glyph/layout coverage.
- Separately authorized device checks: baseline USB tap/drag/type/stop; BLE
  pairing, keyboard, absolute position plus relative click, held drag, scroll,
  Consumer controls and stop/disconnect; then Wi-Fi MCP with USB and BLE input.
- Separately authorized integration: Anan on the same phone with Wi-Fi MCP and
  BLE HID; a computer MCP client with its own observation. Check competing
  sessions, lost replies, mid-drag Wi-Fi/BLE loss, radio coexistence and battery.

Compilation and offline simulation do not establish iPhone compatibility,
notification delivery, end-to-end stop latency, radio performance or endurance.
BLE absolute-pointer behavior is the first real-device gate. New execution
requires explicit authorization for the current target and scope.

First version uses the existing USB setup page to save a 2.4 GHz Wi-Fi network.
The device joins that network as a station and displays its IPv4 address.
Temporary setup hotspots and mDNS are deferred. HTTP LAN credentials have the
same sensitivity as the current USB Bearer export; use a trusted local network.
HTTPS remains USB-only because the existing certificate is scoped to the USB
address. Input selection is explicit, defaults to USB, and is independent of
MCP network access. Changes and reconnects never enable control.

Implementation references are the already pinned ESP-IDF v5.5.5 NimBLE APIs and
TinyUSB descriptor macros; no vendor demo or other application's implementation
is copied. BLE absolute positioning followed by a relative-button report is an
experimental design informed by xhoantran/iphone-use; USB descriptors stay intact.

## Scan-and-select configuration slice

Before implementation, define acceptance: a paired USB-only setup page can start
one bounded asynchronous station scan and poll results; configuration and scans
cannot overlap. An unconfigured station can scan without trying to connect with
empty credentials. Scans pause connection retries; completion/error resumes the
saved network without restoring control. Results are bounded, deduplicated and
exclude hidden/invalid-UTF-8 names, BSSIDs and passwords. The browser uses literal
text/value assignment, supports manual names and resets password when changing
networks. Open-network selection clears/disables password until manual editing.
Pre-implementation browser acceptance uses synthetic responses only; real radio
scan, BLE coexistence, timeout recovery and phone setup remain device checks.
No dependency or MCP tool/schema changes, and no password retrieval is added.
