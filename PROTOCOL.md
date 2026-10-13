# ANAN1 USB protocol

Authoritative first-prototype transport contract. Implementation: `main/main.c`, `main/usb.c`; iOS client: Anan PhoneUSBConnection (maintained in the Anan repository). Source declarations and unresolved provenance are recorded in [THIRD_PARTY.md](THIRD_PARTY.md). See [client integration](docs/client.md).

USB full-speed composite: CDC NCM interfaces 0/1 and HID interface 2. HID report 1 is an absolute single-pointer mouse (15-bit coordinates); report 2 is a six-slot keyboard (one key used). Prototype VID/PID 303a:4008 are development identifiers, not assigned product identifiers. Device consumes up to 100mA by descriptor; actual board power and iPhone compatibility require device validation.

Static IPv4: device 192.168.7.1/24, iPhone 192.168.7.2/24, no router/DNS/DHCP/IP forwarding. TCP port 28766, one connected client, no Wi-Fi/BLE, TLS or pairing. iOS binds its USB wired interface and local IP. The physical USB link is the prototype trust boundary; do not expose/forward this port onto another network. MAC derives from device eFuse, locally administered unicast; host MAC flips the last bit and is consistent with the NCM descriptor.

## Framing and closed sets

ASCII lines terminated by LF. Device buffer holds at most 299 bytes before LF, rejects control bytes outside LF. Client commands are at most 270 body bytes. IDs are decimal UInt32 in 1…4294967295, strictly increasing within a connection; new connections reset server ordering. Client has one request in flight; no replay/deduplication cache and no persistent IDs.

| Direction | Line | Payload and meaning |
| --- | --- | --- |
| Device → app | `ANAN1 READY` | Connection established and prior action worker idle |
| App → device | `PING` | Liveness heartbeat; sent every 300ms |
| Device → app | `ANAN1 PONG` | Heartbeat reply; no action outcome |
| App → device | `STOP` | Invalidates connection generation and closes TCP; interrupts queued/current input |
| App → device | `<id> TAP <x> <y>` | Move, left-button press and release |
| App → device | `<id> DRAG <x> <y> <toX> <toY> <durationMs>` | Move, press, interpolate, release; duration 100…2000ms, excluding report/press overhead |
| App → device | `<id> KEY <usage> <modifiers>` | Keyboard usage 4…231, modifier byte 0…255; press then release |
| App → device | `<id> TYPE <hex>` | 1…120 bytes, lowercase hex, US keyboard printable ASCII plus TAB/LF |
| Device → app | `<id> DONE` | HID reports completed and release attempted; not proof of target-app success |
| Device → app | `<id> ERROR` | Busy/rejected/interrupted HID execution; partial effects possible, observe before retry |

All coordinate integers 0…65535 map to HID 0…32767 by right shift. iOS tools accept normalized 0…1, x left→right and y top→bottom in the upright screenshot. Portrait only. No touch digitizer, pressure, multi-touch, Unicode/IME or clipboard protocol. Modifier bits: LCtrl1, LShift2, LAlt4, LGUI8, RCtrl16, RShift32, RAlt64, RGUI128. Keyboard layout must be US for TYPE.

## Ownership, ordering, cancellation and errors

Server owns a volatile connection generation; each action copies it. Single action worker owns HID reports. One-slot queue; busy rejects another action with ERROR. IDs consumed even by busy rejection. Parser errors, nonmonotonic IDs and overlong/invalid lines close the connection without a result. The worker releases mouse/keyboard after each action and after generation change when USB is available. HID readiness/completion each bounded at 200ms. Release is best effort if the cable/host disappears.

STOP, TCP closure, USB detach/suspend, either physical front button or 1500ms without a heartbeat invalidate input. The network loop polls at 100ms; the worker checks generation/liveness between reports and in 5ms pause increments. These are implementation bounds, not measured end-to-end latency guarantees. A stalled HID call may take its timeout to return.

Client greeting/action timeout is 8s. Timeout, cancellation or disconnect never causes automatic reconnection or retransmission. Any dispatched action without a successful result has an unknown outcome; previous effects are not rolled back. User reconnects, starts/rechecks sharing, enables a new task and observes before issuing fresh input. DONE also requires visual observation. No protocol state, command text or authorization persists in firmware flash. UART logs contain startup/SDK diagnostics, not command payloads.
