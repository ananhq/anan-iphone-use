# Roadmap and handoff

English · [简体中文](roadmap.zh-CN.md)

| Stage | Scope and exit |
| --- | --- |
| A: migration | Independent source/docs, source snapshot, independent build/static evidence; local repository name provisional |
| B: baseline | Agree acceptance scope, resolve source handling, perform only approved validation and form baseline commit; runtime gaps remain explicit |
| C: MCP | Discuss/confirm version, transport, ownership, HTTP trust and client reachability; independently implement, validate interoperability and commit |
| D: display | Review native screen/buttons/touch/power, design/confirm UI, then implement in stages without delaying stop |

A local migration/build is not an accepted stable baseline. No public remote is configured. Keep the old snapshot recoverable; Anan retains only migration/integration pointers rather than another maintained firmware tree. Publication address/tag/commit pins must be assigned before public integration links or Releases.

Current user decision: create the first local checkpoint after repository
preparation and their reported successful reflash. This narrower checkpoint is
not full baseline acceptance; input/stop checks and source-origin resolution
remain pending. See [baseline acceptance](baseline-acceptance.md) and
[release preparation](release-preparation.md). Wireless and MCP feasibility has
been discussed; no implementation/version/dependency choice is accepted by this
preparation. Keep the USB path for later wireless work. Display UI remains a
separate later stage.

## Parallel work

Firmware, protocol and board docs belong here. Anan owns phone capture, client transport, tools, permission, background execution and model integration. Read AGENTS.md and the snapshot before work; preserve the original input hashes through the migration. Do not reintroduce a firmware copy under Anan. Coordinate any ANAN1 changes with the Anan client before implementation; no default submodule/download service.

After the baseline gate, independent research can be split by file ownership/worktree. MCP changes should reuse the current HID execution core. Screenshot/observe is a client capability, not a device tool. USB networking does not make a computer/remote agent reachable. Anan currently accepts HTTPS MCP only, and its pinned SDK version support must be checked before choosing a protocol; do not assume an HTTP `/mcp` endpoint will work. Explicit control authorization, ownership, stop, heartbeat, length/concurrency/origin and cancellation/no-replay rules need a confirmed plan. No version/server/UI choice is made by this migration.

The device UI may show actual connection/action/error/stop. Model-thinking and full task state require a new client-display contract; displayed status must never grant control or extend permission. Hardware has a touch screen, but this firmware implements neither driver. No graphics library/animation is approved yet.

## Contribution and release

Agree a concrete issue/scope before external contributions and preserve origin notices. Changes require relevant bilingual docs and appropriate compilation/static checks; device/E2E permission is scoped per task. Before a public release, complete provenance/dependency notices and final project address, define device support honestly and obtain publication authorization. No license change is implied by extraction.
