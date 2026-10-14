# Wireless setup acceptance scope

English · [简体中文](wireless-setup-development.zh-CN.md)

Agreed before implementation: temporary WPA2 SoftAP for first configuration and
manual network management, five saved 2.4 GHz networks with automatic cycling,
and the same `stopwatch.local` HTTP MCP URL over USB and Wi-Fi. Preserve USB,
seven tools, pairing, explicit grants, stop, input timings and no replay.

Offline scenarios: migrate the previous single-network record without changing
its password; validate stored bounds; update duplicate names; reject a sixth
network without eviction; remove one network; browser scans render names as text
and never store passwords; hotspot exports a stable URL before finishing setup;
Host/Origin restrictions remain tied to actual local interfaces. Native lwIP mDNS
comes from the pinned SDK, with its complete Verisure BSD notice retained alongside the existing lwIP notice; no added package.

Device acceptance, separately authorized: first boot with no network, join the
screen-displayed WPA2 hotspot, automatic DHCP, physical browser pairing, scan/save,
copy config before closing hotspot, return phone to LAN and BLE; repeat from USB;
reboot persistence; move between two saved networks; wrong-password fallback;
full network list and selective deletion; expire/cancel setup; check `.local` on
iPhone and Mac over each interface; duplicate-name conflicts; overlapping subnets;
stop during disconnect, no restored grant or replay. No Internet forwarding or
permanent hotspot input is promised. QR/captive portal is deferred; use the
screen password and Safari URL for this smallest implementation.

Compilation and offline fixtures do not establish radio coexistence, DHCP/mDNS
delivery, phone UI success, actual switching latency or endurance.

## IPv4-only probe regression scope

Before the correction, extract the actual pinned SDK `mdns_probe_and_announce` function and simulate successful IPv4 sends with unavailable IPv6, successful dual-stack sends, and no IPv4. The existing IPv4-only case fails to finish probing, explaining a source-level failure mode. The correction skips IPv6 probing only when no valid link-local IPv6 address exists; keep the three IPv4 probes and conflict checks. Run the same fixture on the generated SDK adapter, compile/package, preserve input/schema/USB files, and separately verify numeric versus `.local` Safari access after an authorized reflash. No claim of measured phone root cause.

## QR acceptance scope

Before implementation: encode a synthetic WPA2 SSID/password, independently decode the resulting image back to the exact Wi-Fi payload, reject empty/invalid names or short passwords and clear stale output, escape delimiter characters, render the complete code with a four-module white quiet zone on the round screen, and independently decode that full static preview. Keep WPA2, DHCP, pairing and hotspot timeout/stop unchanged. Camera recognition and actual iPhone joining are separate device checks.
