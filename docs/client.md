# Client integration

English · [简体中文](client.zh-CN.md)

The service is raw TCP at **192.168.7.1:28766** over USB Ethernet. It is not HTTP or MCP. Bind the USB local address/interface (phone 192.168.7.2/24). Consult [PROTOCOL.md](../PROTOCOL.md) for the authoritative grammar and closed sets.

1. Obtain user control permission in your client. Connect and await `ANAN1 READY` (Anan timeout 8s).
2. Send `PING\n` every 300ms on the same connection. Parse `ANAN1 PONG` independently of action replies. Serialize writes and retain a read loop while waiting for an action.
3. Assign increasing nonzero UInt32 IDs; permit one action in flight. Await its `DONE`/`ERROR`, with bounded timeout. Do not treat PONG as an action result.
4. On stop/cancel/timeout/EOF send best-effort `STOP\n`, close, revoke control and report uncertain partial effects. Do not reconnect/resend automatically.
5. Observe the phone screen before and after input. Reconnect/re-authorize only on an explicit user action.

Illustrative lines (these cause real input if transmitted; documentation does not execute them):

```text
1 TAP 32768 32768
2 DRAG 32768 50000 32768 16000 500
3 KEY 40 0
4 TYPE 416e616e205553422074657374
```

The last line types `Anan USB test`. Normalized coordinates map by the client's established rounding to 0…65535; the device maps them to 0…32767. Keep portrait orientation and verify absolute pointer mapping on the device. Key 40 is Return, not the text character code 40.

The Anan mapping is `phone.tap`→TAP, `phone.drag`→DRAG, `phone.key`→KEY, `phone.type_ascii`→TYPE. `phone.observe` runs in the phone-side ScreenCaptureKit client and does not send a firmware command. The agent, model network, capture, per-Turn permission, attachments and Live Activity remain in [Anan](https://github.com/ananhq/anan); current integration source was uncommitted at migration time, not guaranteed available in a public tag.

An on-phone client can reach the attached USB subnet. A computer-side/remote agent requires an explicit bridge and separate screenshot source; implementing MCP later will not supply either. No generic-agent integration or Android capture support has been validated. Do not expose this unauthenticated input port outside its physical USB trust boundary.
