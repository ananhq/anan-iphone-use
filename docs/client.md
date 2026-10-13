# Client integration

English · [简体中文](client.zh-CN.md)

Anan iPhone Use (`anan-iphone-use`) is a device-side executor. Anan is the first client; other agents may integrate through the public [MCP endpoint](mcp.md). Only M5Stack StopWatch is currently targeted. The client owns observation, inference, planning and user authorization; the board owns USB mouse/keyboard execution.

## Client iPhone Use description

Recommended description for a client tool group that exposes both observation and hardware input (documentation only; not the current Anan implementation):

> Observe and interact with this iPhone. Capture the shared screen and use connected hardware to click, drag, press keys, and type ASCII text. Requires user-enabled control for the current task. Observe after actions to verify their effects.

`observe` / Anan's `phone.observe` is provided by the phone client through system screen sharing, not by this firmware or its MCP tools. `tap`, `drag`, `key` and `type_ascii` execute on the hardware. HID `DONE` or an MCP tool result confirms an input sequence, not a target operation's success; capture a fresh screen and verify the effect after each action.

An on-phone client and a computer-side agent have different network access. The attached phone can use its USB subnet; a desktop/cloud agent needs an explicit route/bridge and its own screenshot source. Supporting MCP alone supplies neither. Third-party interoperability remains unverified. Actual Anan tool-description code belongs to the separate Anan repository/session and is not changed here.

At this documentation review, the read-only Anan source snapshot in `apps/ios/Anan/Tools/Phone/PhoneTools.swift` filters the native group to `phone.observe` and rejects native input tool dispatch. Thus the description above is the recommended combined capability, not a claim that all native Anan phone tools are currently exposed. Use firmware MCP tools for hardware input. The old Anan direct TCP input path is retired; the inspected observation readiness does not require an input connection.

## MCP integration

1. Follow [pairing/import](mcp.md). Use `http://192.168.7.1/mcp`, or optional HTTPS8443, with the board's Bearer header. Import through Anan MCP settings; do not use its old direct StopWatch TCP connection entry.
2. Initialize MCP, retain the session/version headers, send initialized notification and discover the six tools. Status/discovery do not authorize phone input.
3. On StopWatch tap Start control. The first input session owns that grant. `tap`, `drag`, `key`, `type_ascii` execute USB HID; `device_status` and `stop` inspect/revoke control. See [schemas and session rules](mcp.md).
4. Supply normalized portrait coordinates 0…1, US-layout ASCII up to120 bytes, keyboard usages4…231 and modifier bits0…255. Do not send the historical ANAN1 line format.
5. Observe before and after actions. Completion confirms input dispatch, not app success; stop/cancel and inspect unknown partial results instead of replaying. USB loss/reboot revoke control, and no reconnect restores input permission.

From `0.4.0-mcp`, port28766, ANAN1 READY/PING/STOP and direct/MCP source switching no longer exist. [PROTOCOL.md](../PROTOCOL.md) is retained solely for historical builds, not a current integration option. USB Ethernet and HID still use the same cable; supporting MCP does not provide screenshots or a route from desktop/cloud to that USB subnet.
