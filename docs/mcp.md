# Device MCP

English · [简体中文](mcp.zh-CN.md)

Implemented development firmware; compilation/static evidence is separate from
phone, TLS, display and interoperability acceptance. MCP is the sole current control protocol. [PROTOCOL.md](../PROTOCOL.md) is
retained unchanged for historical ANAN1 builds; port28766 is removed. This
document describes the current MCP endpoint. The firmware runs no agent and captures no screen. This repository does not modify [Anan](https://anan.sh).

Anan iPhone Use (`anan-iphone-use`) supplies device-side USB input execution. Client screen observation, inference, planning and user authorization remain separate; see the [client iPhone Use description](client.md). This endpoint has no `observe` tool. The user reports satisfaction with MCP; this does not replace versioned interoperability/stop acceptance evidence. HTTP80 `/mcp` is now implemented alongside optional HTTPS8443. The user reports Anan HTTP support; this firmware does not modify that client.

## First setup: HTTP, no certificate installation

1. Connect the unlocked iPhone to M5Stack StopWatch. Configure USB Ethernet as `192.168.7.2/24`, with no gateway/DNS, and enable AssistiveTouch; see [setup](setup.md).
2. On StopWatch tap Settings → Connect MCP app. Chinese/English selection is remembered.
3. In Safari open `http://192.168.7.1`, tap Start pairing, compare the six-digit code and tap Confirm pairing on StopWatch. The setup window lasts ten minutes.
4. After confirmation, the same page generates MCP JSON automatically. Copy or download it and import into Anan or another HTTP-compatible MCP client. The exported endpoint is `http://192.168.7.1/mcp`, with a per-device Bearer header. No profile installation or system certificate trust is required.
5. On StopWatch tap Imported, go back, then Start control. Control stays enabled until stop/detach/reboot or another documented revocation event. Discovery/status do not enable input. Touch during control or either front button stops and consumes that contact.

Plain HTTP has no TLS encryption and is restricted to the existing exact USB addresses/Host/Origin. HTTPS remains optional under the collapsed page section; it exports `https://192.168.7.1:8443/mcp` from the HTTPS configuration page and retains the prior certificate/trust requirements. Both transports share the same seven advertised tools, sessions and HID executor. HTTPS startup is no longer a prerequisite for HTTP control.

The page retains only its temporary pairing proof in tab session storage; configuration/token stays in the page/download, not local storage. On HTTP, copying uses a browser fallback when the secure-context clipboard API is unavailable; select/copy or JSON download remains available. Actual Safari clipboard behavior is a device check. After closing/expiry/restart, reopen Connect MCP app and start again. Config export requires physical confirmation, a valid ten-minute setup window and the temporary HttpOnly, SameSite Strict setup cookie. A fresh physical setup may export an existing token.

## Repeat as a new user after HTTPS setup

1. Flash the identified HTTP candidate, then reset the board normally; do not erase the whole chip.
2. On iPhone open Settings → General → VPN & Device Management, select only **StopWatch Local CA** and Remove Profile. Enter the iPhone passcode if requested. Do not remove unrelated profiles. See [Apple profile guidance](https://support.apple.com/guide/iphone/install-or-remove-configuration-profiles-iph6c493b19/ios).
3. Delete the old StopWatch HTTPS connection from the client's MCP settings and close the old setup browser tab. Old downloaded profile/config files can be deleted separately.
4. On StopWatch open Settings → Reset pairing, then hold the red confirmation button for two seconds. This clears identity/CA/token and interrupts control; it preserves language preference. Do not reset all iPhone network settings.
5. Repeat the HTTP first-setup steps above. Check that imported JSON uses `http://192.168.7.1/mcp`, not HTTPS8443. A client connection/status check is separate from real phone input verification.

Identity remains board-generated in the dedicated `mcp_kv` NVS partition at `0xf00000`, outside normal merged images. Ordinary upgrades preserve it; whole-chip erase, factory recovery or a different partition layout can remove it. Reset clears only the identity namespace, not language; no commands/actions/grants persist.

The existing ECC P-256 CA/server certificate generation is retained for optional HTTPS and compatibility, including its renewal path. HTTP does not require the phone to trust or install that CA. No Wi-Fi, cloud, OAuth, mDNS or shared signing key is added. Storage remains ordinary NVS; Flash encryption/Secure Boot are outside this slice.

## Endpoint and tools

`http://192.168.7.1/mcp` and optional `https://192.168.7.1:8443/mcp` implement MCP **2025-11-25** Streamable HTTP,
JSON POST responses, authenticated GET 405, DELETE session termination, and
202 empty notification responses. There is no SSE replay, task extension,
resources, prompts, sampling, screenshot or automatic input retry. The initial
implementation is reachable from the attached phone only; a desktop/cloud MCP
client needs another network path, which is outside this wired slice.

| Tool | Required arguments | Meaning |
| --- | --- | --- |
| `device_status` | None | USB, mode, busy and physical grant state |
| `tap` | `x`,`y`: 0…1 | Default click; count=2 double-clicks; holdMs>0 long-presses |
| `drag` | `points`: 2…32 coordinates; `durationMs`: 100…2000 | Two-point drag or continuous path, with optional pre-hold |
| `press_key` | `key`: name enum | Named keyboard/editing and system/media keys |
| `type_ascii` | `text`: 1…120 US-layout ASCII bytes plus TAB/LF | No Unicode/IME |
| `scroll` | `x`,`y`,`vertical` | Bounded wheel over a target; not pixel units |
| `stop` | None | Revoke authorization, interrupt input and request release |

Candidate `0.5.1-mcp` advertises seven tools; optional parameters, limits, progress and hidden compatibility calls are in [HID tool reference](hid-tools.md). Firmware accepts old calls but excludes them from discovery; refresh after upgrading. No new phone bindings or Full Keyboard Access required.

Unknown arguments are rejected. Tool completion confirms a HID sequence, never
an app/task result. Failure/cancellation/lost replies can have partial effects;
observe the phone independently and do not replay automatically. `phone.observe`
is Anan's separate native tool, not an MCP tool on the board; its client screen-sharing/ownership requirements are separate from device MCP input.

## Sessions and safety boundaries

Send `initialize` without a session header, accept the negotiated version, retain
`MCP-Session-Id`, then send `notifications/initialized`. Every later POST includes
that session and `MCP-Protocol-Version: 2025-11-25`. Authorization is a 256-bit
per-device bearer token on every MCP request. Session IDs are not credentials.
There are four sessions, each expiring after ten minutes without accepted use.
Request IDs are opaque nonempty strings up to 64 UTF-8 bytes or safe integers.
Batches, null/fractional IDs, duplicate JSON keys and NUL strings are rejected.

One HID worker serves HTTP/HTTPS MCP with no input queueing beyond its single
slot. Default boot mode is MCP, with control disabled. A fresh physical enable
grants input permission; no source selection is needed. The first dispatched session owns that grant. A different session
requires stop and another physical enable. Permission has no time limit and ends on stop, USB loss, mode change,
credential maintenance, display failure or reboot. Grants are never persisted. No reconnect restores permission or resends input.
Each dispatched input is bounded by eight seconds and its cancellation/device
and authorization generations. A session retains 64 input-ID hashes; duplicate
input IDs and a full ledger are rejected instead of silently replaying. At the
limit create a new session and physically re-enable control.

Long input responses use IDF async request ownership, letting the HTTP task
process status/stop/cancellation while HID runs elsewhere. Cancellation matches
the authenticated originating session and JSON-RPC request ID. Button checks
run separately at 5ms intervals; raw presses stop, while menu/enable decisions
are debounced. USB loss, mode change, explicit stop and
credential maintenance interrupt input. Release after detach is best effort;
no previous effects are undone. Measured stop latency remains a device check.
The server does not infer cancellation solely from a disconnected HTTP response:
clients should send cancellation/stop; physical stop and the eight-second per-action limit still apply. Lost results are unknown, never safe-to-retry guarantees.

The legacy ANAN1 listener/parser/heartbeat and source-selection UI are removed
from `0.4.0-mcp`. Native phone observation is client-owned; the inspected Anan
observe readiness does not require the old input connection. From `0.5.0-mcp`, the Consumer descriptor/report ID 3 is appended; original mouse/keyboard IDs and legacy HID timing are retained. Current `device_status.mode` is `mcp` or
`setup`; there is no `usb` legacy control mode.

Both services require local USB IP `192.168.7.1`, peer `192.168.7.2`, exact Host
and, when supplied, same-origin Origin. No CORS access is granted. HTTP serves setup/config export and the authenticated MCP endpoint. Body size is 4096 bytes, JSON nesting ≤16;
headers ≤2048 bytes, receive/send waits two seconds. These are bounded local
services, not a public internet endpoint. Requests, secrets and device identities
are not deliberately logged; SDK diagnostics must also stay out of public artifacts.

## Source and acceptance

[Module ownership](../main/README.md), [third-party notices](../THIRD_PARTY.md),
[development decisions and predefined checks](mcp-development.md). Panel init
uses a pinned, attributed M5Stack MIT command table and the pinned Espressif
CO5300 driver; native UI uses CST820B touch, own ASCII glyphs and a licensed Chinese font subset, without LVGL. Failure
to initialize/transfer the display prevents MCP enable and pairing confirmation;
raw physical stop remains available; there is no legacy input fallback. Panel placement/power and
phone trust/interoperability require separately approved device verification.

## Blank-screen startup diagnosis

Touch Settings once the screen is showing status. Long-press yellow A for 0.7 seconds is a physical fallback.
The screen should initialize at boot; holding A is not its power-on operation.
After flashing, short-press the power button to leave download mode/reset.
The initial MCP candidate received a user report of a blank screen; root cause
and reset state were not confirmed. The next candidate adds M5IOE1 wake/retries,
transaction settling, sleep disable and GPIO/power read-back checks. This is a
source-based startup correction, not a verified device fix.

On that candidate, open `http://192.168.7.1/status` from the attached phone to
read `firmwareVersion`, `displayStage`, `displayError` and `usbOnline`. It is a
USB-only, no-store, read-only diagnostic endpoint without identities or secrets;
it cannot enable or send input. The setup page also shows this information.
An error identifies IO-expander discovery/configuration, screen power, DMA
allocation, SPI/panel initialization or pixel transfer. A `ready` state confirms
the software transfer completed, not that light/pixels were physically observed.

The `0.1.2-mcp` candidate corrects USB socket-address checks for IDF's IPv6
dual-stack listener: IPv4-mapped IPv6 addresses are compared against the same
exact device/phone IPv4 pair. Native IPv6/other peers remain rejected. HTTP Host
and Origin permit the equivalent explicit default port `:80`; HTTPS still uses
exact `:8443`. Rejections now identify the failed address/Host/Origin category
without echoing request values. This addresses a source-confirmed defect behind
the user's `USB origin/host required` report; phone acceptance still needs a retry.

## Touch and bilingual operation

See [touch UI decisions and predefined checks](touch-ui.md). Home shows MCP
control, one start/stop button and Settings. There is no Anan direct/source
selection; the same USB cable still carries both Ethernet and HID.
For fallback, yellow A selects/highlights a visible button, blue B activates it;
a long yellow press opens Settings. A press that stops control is consumed until
release. Touch enables only on a fresh release over the same visible button;
failed reads, moved-out contacts, stale frames or control changes cancel the tap.
Reset alone uses a two-second hold. Touch I2C polling and display rendering are
separate from the I2C-free, higher-priority physical stop task. The remembered
language changes device UI; the phone page also has its own bilingual switch.
Touch coordinate mapping, failure recovery, language persistence and latency
remain real-device acceptance items, despite compiled/static preview evidence.

In `device_status`, `controlEnabled` remains a boolean.
`controlSecondsRemaining` is `null` while enabled (no time limit), otherwise `0`.
MCP session idle expiry and the eight-second per-action limit are unchanged.
