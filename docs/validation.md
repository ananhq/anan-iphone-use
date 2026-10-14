# Migration validation

English · [简体中文](validation.zh-CN.md)

## Defined acceptance scope

This migration authorizes local extraction, documentation, compilation and static checks. It does not authorize flashing, phone/HID/model operations, simulator/E2E or publication. Before implementation, the checks were defined as: preserve original runtime/build inputs by SHA-256, independently build/package outside [Anan](https://anan.sh), check target/offset/Flash/lock/source manifest, check bilingual links/ignore rules and record provenance gaps. A–H device scenarios from Anan are not automatically approved here.

[source-snapshot.json](../artifacts/migration/source-snapshot.json) identifies the original 16 files; all 12 original build inputs matched the historical image manifest. Runtime source, scripts, lock and wire behavior are unchanged. Root CMake pins the original IDF app version `ec6bb29-dirty`, previously inferred from Anan Git; this is the only build-configuration adjustment. Added module README files are extra manifest entries because package.py hashes all main/ files; they are not compiled sources.

## Evidence levels

| Evidence | Status |
| --- | --- |
| Original image | 455840 bytes, SHA-256 `5e3e96b3bf2e1699c6406874de66a85e7efa1e3823ec83738f4971b0f2d8e230`; source hashes checked at extraction |
| Historical flash | User supplied esptool successful-write/hash output; not a new migration device run |
| Historical behavior | User reported Ethernet, static IP, connection, preview, observe and phone input working |
| Whole task/endurance | Notes task difficult; background improvements reported on Anan; no full firmware acceptance trace |
| Migration build/static | See [build-check.json](../artifacts/migration/build-check.json) for commands/tool versions/results; not device acceptance |
| Provenance | Partial review, USB implementation origin unresolved |
| Local checkpoint | Explicitly authorized after the user's successful reflash report; full input/stop acceptance and source resolution remain pending |

Generated binaries, components, sdkconfig and raw machine-local build logs stay ignored. Committed evidence stores sanitized outcomes/source hashes, not private paths, serials, screenshots or credentials. Cross-machine binary identity is not promised.

## Baseline preparation

Preparation evidence is [preparation-check.json](../artifacts/baseline/preparation-check.json).
The user subsequently supplied esptool successful-write/hash-verification output
and explicitly requested the first local commit. The sanitized current scope is
recorded in [checkpoint.json](../artifacts/baseline/checkpoint.json).
Migration records above remain historical. The candidate's manifest is generated
under ignored `build/`; use the [baseline procedure](baseline-acceptance.md) to
report results against that exact image hash. Phone input/stop acceptance and
source-origin resolution remain pending; neither a stable baseline nor public-release readiness is
claimed. [Release preparation](release-preparation.md) lists the remaining gates.

## Next device acceptance, separately authorized

Use the migrated image hash and synthetic notes: Ethernet/HID enumeration and 60s READY/PONG; full-screen portrait preview; observe-only; one tap/ASCII/drag with visual confirmation; stop/button/detach during input and no new actions/replay; no-Mac multi-step closure. Record board revision/OS/client/model only as appropriate, preserve failures and unknown partial effects. Store repeatable sanitized artifacts under `artifacts/e2e/<scenario>/<run-id>/` if E2E is approved. Until that run, keep deviceVerified=false.

## MCP development candidate

[build-check.json](../artifacts/mcp/build-check.json) records the separate
`feat/device-mcp` current candidate. The original `0.1.0-mcp` build remains in
[initial-build-check.json](../artifacts/mcp/initial-build-check.json). Compilation/packaging and static
configuration, original-action-body equivalence, embedded JSON/JavaScript,
DER structure and documentation checks passed. `usb.c` and PROTOCOL.md match
the local checkpoint byte-for-byte; new arbitration/UI/identity code is separate.
[Device MCP](mcp.md) documents setup and runtime limits. No new flash, phone,
display, TLS trust, interoperability, latency or endurance run was performed.
Runtime heap peak is unmeasured. Existing provenance and acceptance gaps remain.

The user subsequently reported a blank display; exact flashed image/reset state
were not established. Candidate `0.1.1-mcp` adds expander wake/settling/sleep
configuration and verified power writes, plus read-only USB startup diagnostics.
Its compilation/static checks are recorded separately from that report; no
verified device fix or new device action is claimed.

The user then reported the screen lighting and `USB origin/host required` while
accessing the phone setup flow. Exact image/flash trace was not supplied.
Pinned IDF HTTP/lwIP source plus current IPv6 configuration explain a separate
address-family defect: the dual-stack listener maps IPv4 into `AF_INET6`, while
the adapter formerly accepted only `AF_INET`. Candidate `0.1.2-mcp` normalizes
both representations to the unchanged exact USB addresses, permits equivalent
HTTP default-port spelling, and returns fixed rejection categories. Static/build
results are recorded; successful phone enrollment/trust/MCP input remains pending.

Candidate `0.2.0-mcp` implements the user's touch-first and device bilingual
switching request. It retains the USB/ANAN1 source contract, adds a native
CST820B adapter, shared widget layout/hit regions, frame/gesture authorization
checks and separate language storage. Compilation/package, both-language glyph
coverage, 22 static document previews and layout/text bounds passed. The font
source/derivation/OFL notice is recorded. These synthetic previews are not phone
or board screenshots. Touch, actual stop latency, preference persistence and
MCP phone acceptance remain unverified; no new device action was performed.

Candidate `0.2.1-mcp` implements the user-approved removal of the three-minute
control grant. Grants remain volatile and stop/detach/reboot-revoked; other
revocation paths and session/action bounds remain. Build/static/layout checks
are recorded separately; no longer-than-180-second device run is claimed.

The preceding touch build is preserved in
[touch-ui-build-check.json](../artifacts/mcp/touch-ui-build-check.json);
[build-check.json](../artifacts/mcp/build-check.json) now identifies `0.2.1-mcp`.

Candidate `0.3.0-mcp` adds HTTP80 MCP and physically confirmed same-page config
export while retaining optional HTTPS. HTTP onboarding no longer requires iOS
certificate trust. Compile/package, static both-language layout/glyph and
JavaScript/source/links checks are recorded in the current MCP build record.
The preceding continuous-control build remains in
[continuous-control-build-check.json](../artifacts/mcp/continuous-control-build-check.json).
No new flash, certificate removal, board reset, HTTP phone interoperability or
clipboard/device run was executed by the agent; those remain user device checks.

The user subsequently reports the HTTP first-user flow verified and working
well; see [sanitized feedback](../artifacts/mcp/http-user-feedback.json). This
is a user report, without an independently checked image hash or full scenario
trace; it does not establish acceptance of the next candidate.

Candidate `0.4.0-mcp` removes ANAN1 TCP28766, parsing/heartbeat/legacy arbitration
and control-source UI. It retains USB/HID input sequences and initializes MCP
without automatically granting control. The prior HTTP build is preserved in
[http-onboarding-build-check.json](../artifacts/mcp/http-onboarding-build-check.json).
Source/symbol/build/package/document-render checks are separate from a new
flash/device run; current MCP-only acceptance remains pending.

## HID expansion candidate

Candidate `0.5.0-mcp` implements thirteen MCP tools including seven named keyboard/scroll/gesture/Consumer tools; no client changes or new phone bindings/FKA setup. Existing mouse/keyboard IDs and legacy executor/sender bodies are retained; Consumer ID3 is appended. [Predefined acceptance](hid-development.md) and first-party offline fixtures preceded implementation. Full pinned-IDF compilation/packaging and simulated input/cancellation/progress checks passed; [sanitized record](../artifacts/hid/expansion-build-check.json) separates them from device behavior.

No flash, phone input, provider, client connection or E2E was executed. Consumer enumeration/mappings, wheel/gesture recognition, new-tool Anan interoperability, stop latency and endurance remain device-unverified. Prior provenance and acceptance gaps remain; historical `usb.c` byte-identity statements above apply to earlier candidates.

## Tool consolidation candidate

Candidate `0.5.1-mcp` advertises seven tools, merging stationary gestures into `tap`, held drags/continuous paths into `drag` and named keyboard/Consumer keys into `press_key`. Firmware retains hidden legacy names and old drag arguments; clients must refresh discovery and may locally reject old names or new parameters. Default tap and unheld two-point drag retain prior timing. Descriptions are concise; shared rules appear in both server description and initialize instructions. Read-only inspection found Anan consumes the server description, so the design does not depend on its consuming instructions.

Predefined consolidation scenarios passed with simulated reports/time; seven schemas/full key enums, unchanged executor/USB/power/grants/stop/dependencies and bilingual documentation checks passed. Full pinned-IDF compile/package outcomes are in the [current record](../artifacts/hid/build-check.json); the prior [expansion record](../artifacts/hid/expansion-build-check.json) is preserved. No flash, phone actions, client connection, provider or E2E ran; merged Anan discovery/calls and device behavior remain unverified.

## Wireless candidate

Candidate `0.6.0-wireless` adds Wi-Fi station HTTP MCP and experimental BLE HID,
explicit input selection and USB-first Wi-Fi configuration. Pinned-IDF compile,
package, unchanged USB sender/descriptor checks, seven-schema comparison,
predefined offline routing/payload scenarios, existing HID scenarios, browser
script syntax and 28 bilingual static layouts passed. Report mutex behavior and
radio/phone delivery are not established by host simulation. See
[predefined scope](wireless-development.md), [setup](wireless.md) and
[sanitized build record](../artifacts/wireless/build-check.json). No flash,
Bluetooth/Wi-Fi device probe, phone input, client/provider or E2E was executed.

## Wi-Fi scan candidate

Candidate `0.6.1-wifi-scan` adds paired USB-only asynchronous scans, bounded and
deduplicated names, scan/select/manual setup and open-network password handling.
Acceptance was defined before implementation. Pinned-IDF compilation/package,
existing offline HID/routing/report scenarios and synthetic browser selection,
literal hostile SSIDs, manual/open-network and failure recovery checks passed.
No real Wi-Fi scan, flash, phone action or E2E was executed. See
[scan record](../artifacts/wireless/scan-build-check.json). Radio timeout/coexistence
and reconnect behavior remain device-unverified.

## Wireless setup candidate

Candidate `0.7.0-wireless-setup` adds temporary WPA2 SoftAP/DHCP configuration, five-network storage with v1 migration and retry cycling, and one fixed HTTP hostname on USB/Wi-Fi through the already pinned lwIP responder. [Acceptance](wireless-setup-development.md) and storage/authority/browser scenarios were defined before implementation. Pinned-IDF compilation/package, existing HID/routing/report scenarios, migration/capacity/removal and exact-Origin fixtures, USB/hotspot synthetic scan UI, JavaScript syntax, 32 static bilingual layouts, preserved seven schemas/USB HID sender bodies and manifest checks passed. USB now adds IGMP; sdkconfig reserves one mDNS client slot. No dependency upgrade or project license change; the additional Verisure BSD notice is retained.

The [sanitized record](../artifacts/wireless/setup-build-check.json) marks flashed/deviceVerified false. No serial/radio probe, phone input, live client/provider, simulator or E2E was executed. The user reported the prior wireless flow working; it does not establish this candidate's DHCP/mDNS, AP+STA scanning, multi-network timing, radio coexistence, runtime heap, grant/stop or endurance acceptance. Numeric recovery paths remain.

Candidate `0.7.2-wifi-qr` follows the user's failed `.local` report with a source-reproduced fix for the pinned lwIP timer's mandatory IPv6 probe on IPv4-only netifs, and adds a screen Wi-Fi QR while retaining random WPA2 credentials and physical pairing. Timer regression and QR scenarios were defined before the changes. SDK compile/package, existing offline scenarios, independent QR and full static round-screen decoding passed; [record](../artifacts/wireless/qr-build-check.json). No agent-executed flash, radio/serial probe, phone camera/joining, live client/provider or E2E ran. IPv4 name-conflict behavior and new phone `.local` success remain device-unverified.

## Task-oriented discovery metadata

Candidate `0.7.3-discovery` changes only seven tool descriptions, initialize description/instructions and version, with bilingual usage terms and optional full-group overview guidance. Acceptance was defined first in [tool discovery](tool-discovery.md). Embedded JSON, exact name/order/schema/annotation preservation, MCP code equivalence outside its two metadata statements, preserved input/HID/radio/storage/dependencies, existing offline HID scenarios, pinned-IDF compile/package and manifest/link checks passed. Tool descriptions increased from 1,373 to 1,525 characters; server description is 463 characters, below the inspected Anan 500-character limit. These are character counts, not measured model tokens. See [record](../artifacts/discovery/build-check.json). No client code, new unit tests, flash, device/provider or E2E execution. Search/model adoption and real app-opening outcomes remain unverified; metadata guidance does not load schemas or grant input.

## Bond-preserving flash helper

The user reports reconnection failure after reflashing. The [predefined scope](flashing.md) adds a first-party host helper that splits existing merged images around default BLE `nvs`; partition layout, firmware and BLE security remain unchanged. Predefined offline sector-write scenarios, image/layout rejection, failure propagation and syntax checks passed. The existing candidate was repackaged with refreshed source hashes and identical binary SHA-256. See [record](../artifacts/flashing/check.json). No firmware recompilation, device access or actual iPhone reconnection verification ran; previously erased bonds require one new pairing.
