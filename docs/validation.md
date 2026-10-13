# Migration validation

English · [简体中文](validation.zh-CN.md)

## Defined acceptance scope

This migration authorizes local extraction, documentation, compilation and static checks. It does not authorize flashing, phone/HID/model operations, simulator/E2E or publication. Before implementation, the checks were defined as: preserve original runtime/build inputs by SHA-256, independently build/package outside Anan, check target/offset/Flash/lock/source manifest, check bilingual links/ignore rules and record provenance gaps. A–H device scenarios from Anan are not automatically approved here.

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
