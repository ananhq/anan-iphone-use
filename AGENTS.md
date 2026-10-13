# Repository instructions

- Keep this firmware focused on M5Stack StopWatch. Use the smallest confirmed design.
- Read docs/roadmap.md and docs/validation.md before work. Migration, baseline acceptance, MCP and display UI are separate stages. Discuss material architecture, protocol, dependency and license changes before implementation.
- main/ and build/configuration files own firmware; do not copy the Anan iOS client or Agent here. PROTOCOL.md is the sole authoritative ANAN1 contract; translated summaries must not define a second contract.
- Preserve the existing ANAN1 wire format, HID descriptors, timing, power and stop behavior during migration. No implicit replay or reconnection of input.
- Read THIRD_PARTY.md before copying code. Record exact source versions, file origins and applicable notices; unresolved provenance must remain explicit. Do not change licenses without confirmation.
- Document first-party modules in English and Simplified Chinese. Mark planned capabilities and unverified behavior honestly. Update both versions together.
- Compilation/static/document checks are allowed. Flashing, device actions, live providers, simulators and E2E require explicit approval for the current scope. Never infer that earlier device permission applies. Do not write unit tests after implementation.
- Never include model credentials, pairing records, device identifiers, private screens, machine paths or unfiltered logs in committed artifacts. Keep build/, managed_components/, sdkconfig and caches ignored.
- Form the agreed baseline commit only after its acceptance scope and source handling are confirmed. No push, GitHub repository creation or Release without publication authorization.
