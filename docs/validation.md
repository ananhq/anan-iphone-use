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
