# Wi-Fi MCP and BLE input

English · [简体中文](wireless.zh-CN.md)

Candidate **0.7.6-touch-ui** retains USB, temporary hotspot configuration, screen Wi-Fi QR,
five saved networks and fixed-hostname MCP. Its large-button menus and Home-only estimated
battery are documented in [UI acceptance](history/touch-usability.md). The user reported the previous wireless flow working; that report
has no independently checked image hash or full acceptance trace. This candidate
has compilation/offline evidence only. See [predefined checks](history/wireless-setup-development.md).

## Connections

| Agent → device | Device → iPhone | Purpose |
| --- | --- | --- |
| USB network | USB HID | Retained wired route |
| Wi-Fi HTTP MCP | USB HID | Computer/phone agent reaches a board plugged into the phone |
| Wi-Fi HTTP MCP | BLE HID | Wireless route |
| USB network | BLE HID | Separate BLE from Wi-Fi acceptance |

The seven MCP tools and portrait-normalized coordinates stay the same. Select
USB/BLE input on the device. The client supplies screenshots and verifies effects;
the board neither receives AirPlay nor runs an agent. USB/BLE selection is
independent of the MCP network path.

## First setup without a phone cable

Initial firmware flashing still uses a computer and data cable. With the candidate
installed, phone network configuration can use the device hotspot:

1. With no saved network, StopWatch opens a temporary **StopWatch-setup** hotspot
   and its pairing page. Later, use **Settings → Connect / Wi-Fi → Set up Wi-Fi** to open it manually. Input is stopped and BLE suspended, retaining
   the selected backend and bond. The hotspot closes after ten minutes or when
   you return from setup. It is configuration-only and provides no Internet.
2. Scan the Wi-Fi QR code on StopWatch with your phone camera and choose to join.
   The code includes the random WPA2 password; manual joining **StopWatch-setup**
   using the screen password remains available.
   DHCP supplies the address automatically; no manual IP/mask/DNS is needed.
   If iOS warns that the network has no Internet, stay connected during setup.
   Open **http://192.168.4.1** in Safari. The Wi-Fi QR code only joins the hotspot; open Safari separately.
   Automatic captive-page opening is not implemented.
3. Start pairing in Safari, compare the six-digit code and confirm on StopWatch.
   This authorizes the browser setup session, not phone input. No certificate is
   required. Credentials and Bearer exports must stay on the user's device.
4. Scan nearby **2.4 GHz** Wi-Fi, choose a network and paste its password once.
   Hidden networks can be entered manually. Save Wi-Fi. The device attempts the
   connection while keeping its configuration hotspot open. Radio channel changes
   can interrupt that browser connection; rejoin the same hotspot and reload if
   necessary. The password is not stored in browser storage or returned by APIs.
5. Generate and **copy/download the MCP configuration before closing the hotspot**.
   HTTP exports use **http://stopwatch.local/mcp** and the existing Bearer header.
   Configuration can be exported after saving even while Wi-Fi is connecting;
   exporting it is not proof that the saved network works. Wait for the page's
   Wi-Fi address before finishing, or correct the password/retry if it never joins.
6. Tap **Done / Home** (or Back) on StopWatch to close the hotspot. Rejoin
   your usual Wi-Fi on the phone, then import the copied configuration into
   [Anan](https://anan.sh) or another HTTP MCP client. BLE resumes only if selected;
   control stays disabled. The phone/computer and StopWatch must be on the same
   accessible IPv4 LAN.
7. For wireless input select **Settings → Phone input → Bluetooth LE**, pair **Anan
   StopWatch** on iPhone and enable AssistiveTouch. Return home and tap **Start
   control** once input is connected: animated eyes replace Home; either front key
   stops and returns Home. Only one bonded phone is supported.

The hotspot is a setup tool, not a router or permanent MCP/BLE operating mode.
Phone hotspots, captive-login Wi-Fi and enterprise authentication are not accepted
features. A LAN overlapping `192.168.7.0/24` or `192.168.4.0/24` is rejected.

## USB setup and recovery

The existing [USB phone configuration](setup.md) and `http://192.168.7.1` setup
page remain available. Start **Settings → Connect / Wi-Fi → Connect MCP app**, pair physically,
scan/save or manage Wi-Fi from that page. Returning from setup also closes any
active hotspot. Optional HTTPS and certificate export remain USB-only at
`https://192.168.7.1:8443/mcp`.

Default HTTP exports use the same `stopwatch.local` URL for USB and Wi-Fi.
Changing DHCP address or the selected USB/BLE input does not require a new MCP
configuration. An existing numeric-IP import needs one replacement import to
adopt the fixed hostname. A disconnected client may need to reconnect and
initialize a new MCP session; this is separate from importing configuration.

mDNS is answered by the device on each local interface and resolved by the
computer/phone OS. It is local discovery, not a public DNS name. Guest isolation,
multicast filtering, VPN routes and client resolver behavior can prevent it.
If resolution fails, the numeric IP remains an endpoint/setup fallback, but its
configuration will not track DHCP changes. Simultaneous USB/Wi-Fi resolution and
Anan reconnect behavior still need device acceptance. Use one StopWatch per LAN:
name conflicts are reported by `/status` and block fixed-hostname export. The
candidate does not assign collision suffixes or rename an existing client URL.

## Network memory and switching

- Store up to **five** SSID/password pairs in `mcp_kv`, automatically reading the
  previous single-network format. Saving an existing name updates its password
  and moves it first. A sixth distinct name is rejected; delete a saved network
  first. Nothing is silently evicted.
- Start with the most recently saved network and cycle through saved entries
  after disconnect/failure or a 15-second connection deadline. Retries wait one
  second between attempts; DHCP/radio/event scheduling adds latency. The current
  successful choice is retained in RAM; startup order is not a signal-strength
  ranking. Moving to a previously saved place needs no new password; a new place
  uses the same hotspot setup. No automatic open hotspot appears just because a
  known network is temporarily unavailable.
- Manage names or **Remove selected network / Forget all Wi-Fi** only from a
  physically paired USB/hotspot browser session. `/wifi/networks` never returns
  passwords. Pairing identity and BLE bonds are separate from network memory.
- Neither network reconnection, closing setup nor BLE reconnection restores a
  control grant or replays input. USB/BLE selection remains explicit. Stop,
  selected-input loss, active LAN loss and reboot revoke control.
- BLE bonds live in default `nvs`: reboot and updates through the current
  `flash.sh` / `flash.py` preserve them. A continuous merged-image write from
  `0x0` still erases bonds; use the [flash helper](setup.md). Previously erased
  or inconsistent bonds need **Settings → More settings → Pair Bluetooth → Hold 2s to reset**, forgetting on the phone and
  one new pairing. Updates preserve `mcp_kv` but never restore control grants.

## Implementation and evidence

Candidate `0.7.5-ble-absolute` tests USB's single absolute mouse report over BLE,
after the user reported that `0.7.4` did not fix Notes drawing. BLE report ID4 is
removed; re-pair for this descriptor change and first verify ordinary taps.
See [comparison and acceptance](history/ble-absolute.md). Drawing remains unverified;
USB and the seven MCP tools are unchanged.

Host/Origin validation accepts the exact numeric device address or
`stopwatch.local`, with an exact matching Origin when supplied. USB still requires
peer `192.168.7.2`; STA and hotspot require a peer on their actual IPv4 subnet.
MCP on the hotspot is rejected; authenticated MCP is available on USB/STA.
HTTPS remains numeric USB-only. No Internet forwarding, relay or client changes
are added. Native mDNS uses the pinned SDK's lwIP source, with a hash-checked build adapter
that skips IPv6 probing when no valid link-local IPv6 address exists. The previous
function kept waiting for both families and could never answer IPv4 queries.
This failure was reproduced with the actual SDK timer function in an offline
fixture; it is a source-level correction, not confirmed phone diagnosis. The
Wi-Fi QR encoder uses pinned Nayuki v1.8.0 C code with its MIT notice retained.

Scans retain up to 20 distinct displayable names from the strongest 40 records,
excluding hidden/unsupported types. Scan results contain SSID, RSSI and open
status only. Async scans pause connection attempts and serialize with writes;
waiting is up to eight seconds plus one second for abort completion, with SDK
startup/scheduling extra. Actual AP+STA scanning and BLE coexistence need checks.

After sourcing ESP-IDF **v5.5.5**, build/package without accessing hardware:

```sh
idf.py -B build/wifi-qr-candidate -D SDKCONFIG=build/wifi-qr-candidate/sdkconfig build
python package.py build/wifi-qr-candidate
bash tools/check_wireless.sh
bash tools/check_hid.sh
```

Use the matching image and manifest; don't reuse an older generated sdkconfig.
[Current record](../artifacts/wireless/qr-build-check.json) distinguishes
compilation/offline evidence from device acceptance. Previous
[wireless](../artifacts/wireless/build-check.json) and
[scan](../artifacts/wireless/scan-build-check.json) records remain historical.
No agent-executed flash, radio/device probe, phone action, provider or E2E ran.

The preceding [setup record](../artifacts/wireless/setup-build-check.json) remains historical. QR generation and the full synthetic round-screen preview were independently decoded by ZXing-C++ 2.3.0; actual iPhone camera recognition/joining and `.local` resolution still require device checks.

Synthetic setup preview (sample password only, not a device screenshot):

![Hotspot QR setup](images/setup/en-pair-hotspot.png)
