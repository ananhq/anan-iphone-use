# Roadmap and handoff

English · [简体中文](roadmap.zh-CN.md)

Anan iPhone Use (`anan-iphone-use`) executes iPhone actions on M5Stack StopWatch; clients own observation, inference, planning and authorization. The repository address is [ananhq/anan-iphone-use](https://github.com/ananhq/anan-iphone-use), with private visibility; public distribution remains pending. Naming does not change ANAN1, USB identifiers, tools or historical artifacts. See the [GPL license and source handling](license-plan.md).

| Stage | Status / exit |
| --- | --- |
| Migration | Local source/docs/build checkpoint `4c6015c`; no public remote or Release |
| Baseline acceptance | User reported successful reflash and authorized checkpoint; phone input/stop and source-origin resolution remain pending |
| MCP and required UI | Implemented through candidate `0.4.0-mcp`, compilation/static previews passed; user reports display lighting and MCP satisfaction; scoped touch/persistence/interoperability/stop acceptance remains pending |
| Plain HTTP MCP | Implemented HTTP80 MCP and same-page pairing/config export; user reports separate [Anan](https://anan.sh) HTTP support; user reports the preceding HTTP first-user flow works well; the new MCP-only image remains unverified |
| MCP-only | User-approved removal of ANAN1 port28766/parser/heartbeat and source selection; USB NCM/HID retained, MCP default without input grant |
| HID expansion | Candidate `0.5.1-mcp`: seven advertised tools, compatibility calls retained, retained mouse/keyboard IDs/timing plus Consumer ID 3; no additional phone mappings/FKA; device acceptance pending |
| Wireless | `0.9.9-compact-cache` frees 8,960 static internal bytes without PSRAM after 0.9.8 showed AP allocation failure; retains reset evidence and radio/input behavior. 0.9.7 PSRAM is withdrawn; [scope](history/network-startup.md). Actual headroom and network recovery pending device acceptance |
| Further display | `0.9.3-avatar-edges`: retain responsive motion, align compact windows and every DMA batch to even coordinates/sizes; [scope](history/avatar-responsive.md). User reports 0.9.2 is smooth with fine trails; trail correction and scanout remain device-unverified |

This local checkpoint is not a fully accepted stable baseline. Keep its source
and historical artifacts recoverable. Anan owns capture, agent, native tools,
permissions, background and model integration; firmware owns input and device
configuration. The user forbids Anan code changes for this work and deferred
further observation changes. A simple button/menu UI accompanies MCP setup.
See [MCP decisions](history/mcp-development.md) and [implementation/setup](mcp.md).

[PROTOCOL.md](../PROTOCOL.md) remains the historical ANAN1 contract; the runtime
service is retired at the user's request. MCP is the current sole control protocol and uses the
same HID worker, preserves original mouse/keyboard reports and legacy timing (with Consumer ID 3 appended in `0.5.0-mcp`), has explicit physical grants,
session ownership and no implicit input retry/reconnect. Ordinary identity
storage is separate from control grants. USB reachability and native Anan
observation gates remain limits; MCP supplies neither screenshots nor a network
bridge to remote clients.

Compilation/static checks are allowed. Flashing, device actions, providers,
simulators/E2E need explicit authorization for the current scope. Record their
actual outcomes separately; build success does not establish hardware behavior.
No post-implementation unit tests. No default delegation or second firmware copy
under Anan. Material architecture/protocol/dependency/license changes are
reviewed before implementation. Resolve original provenance and linked-component
notices before distribution; project license changes need confirmation.

Contributions require agreed scope, origin notices and bilingual documentation.
Local commits and publication are distinct. No push, repository creation or
Release without publication authorization. See [baseline acceptance](baseline-acceptance.md)
and [release preparation](release-preparation.md).

The user added touch-first UI and device Chinese/English switching/persistence.
Implemented as a separate slice; see [touch decisions](history/touch-ui.md). This does not
change Anan/ANAN1 or expand device execution authorization at that stage;
the later user-approved MCP-only change retires the ANAN1 service.

Wireless setup candidate `0.7.0-wireless-setup` implements the agreed fixed USB/Wi-Fi hostname, five saved networks and temporary SoftAP provisioning. Current [setup](wireless.md), [predefined scope](history/wireless-setup-development.md) and [validation](validation.md) supersede the earlier USB-first/numeric-IP flow. No new device run or stable acceptance is claimed.

Candidate `0.7.2-wifi-qr` follows the user's failed `.local` report with a source-reproduced fix for the pinned lwIP timer's mandatory IPv6 probe on IPv4-only netifs, and adds a screen Wi-Fi QR while retaining random WPA2 credentials and physical pairing. Timer regression and QR scenarios were defined before the changes. SDK compile/package, existing offline scenarios, independent QR and full static round-screen decoding passed; [record](../artifacts/wireless/qr-build-check.json). No agent-executed flash, radio/serial probe, phone camera/joining, live client/provider or E2E ran. IPv4 name-conflict behavior and new phone `.local` success remain device-unverified.

Candidate `0.7.3-discovery` is the agreed metadata-only discovery slice: bilingual task terms and a soft complete-group overview recommendation, without additional tools/schema or execution changes. Predefined scope and limitations are in [tool discovery](history/tool-discovery.md); model adoption and phone outcomes are not inferred from compilation/static evidence.
