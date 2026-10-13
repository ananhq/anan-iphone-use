# Open-source release preparation

English · [简体中文](release-preparation.zh-CN.md)

This is a local preparation record, not a public release or a completed origin
audit. The repository currently contains firmware, pinned build inputs,
bilingual documentation and sanitized migration evidence. The public project
address is not assigned. Existing license declarations are retained.

## Local baseline sequence

1. Organize documentation and check source equivalence, links, ignored outputs
   and independently compiled/packaged inputs.
2. Hand the identified candidate image to the user for a fresh flash and
   [manual verification](baseline-acceptance.md).
3. The user authorized an initial local checkpoint after their successful
   reflash report. Keep input/stop acceptance and source gaps explicitly pending;
   confirm full baseline scope and source handling separately.
4. Continue separately agreed wireless/MCP research and implementation after
   that checkpoint. Keep the current USB path available.

## Before public source distribution

| Item | Current status / required action |
| --- | --- |
| Source origin | `main/usb.c` overlaps the inspected actell USB implementation; exact origin is unresolved. `board.c` register implementation and historical generation/reuse also have recorded gaps. Resolve and record source handling before public distribution |
| License and notices | Existing Apache-2.0 declarations and third-party notices are present. Any different license/source obligations require a confirmed decision; attribution alone does not resolve derivation |
| Baseline | Initial local checkpoint authorized after the user's successful reflash report; full input/stop acceptance and source-origin resolution remain pending |
| Repository identity | Confirm final project name/address; replace provisional identity in NOTICE and docs and assign public commit/tag pins before publishing integration links |
| Public support claims | Limit support to StopWatch; describe input limitations and actual verified client/OS versions. Generic ESP32, wireless, MCP, display/touch and endurance are not accepted capabilities |
| Publication | Public repository creation, push and release each require publication authorization |

[THIRD_PARTY.md](../THIRD_PARTY.md) remains the source-origin record. Do not
remove its unresolved findings merely to make a README appear release-ready.
Device verification cannot establish implementation ownership or licensing.

## Additional requirements for a binary release

- Inspect the actual linked application and bootloader components, including
  applicable SDK, library and ROM/proprietary notices. The existing four notice
  copies are not an exhaustive binary-license inventory.
- Build the release from its identified source commit and pinned toolchain;
  record image/source/lock hashes, offsets, Flash size and scoped validation.
  The historical IDF app version `ec6bb29-dirty` is retained for this baseline;
  a release version change is a separate decision.
- Include applicable notices and recovery/setup instructions in the distribution.
  Never include raw machine-local logs, private configuration or device identifiers.
- Confirm the candidate actually tested is the image being distributed. Rebuilds
  require a hash comparison; compiler success does not inherit device acceptance.

Generated files remain under ignored `build/` and `managed_components/`.
Migration evidence under `artifacts/migration/` is historical and stays intact;
new baseline evidence belongs under `artifacts/baseline/`. See
[validation](validation.md) for the evidence boundaries.
