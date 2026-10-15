# Architecture illustrations / 架构示意图

Generated with the built-in imagegen tool; illustrative diagrams, not device screenshots.
使用内置 imagegen 生成，为架构示意图，非设备截图。

- [English](iphone-agent.en.png) / [中文](iphone-agent.zh-CN.png): the approved wireless flow, with Anan on iPhone and MCP Server on StopWatch. 已确认的无线流程：Anan 在 iPhone 上，MCP Server 在 StopWatch 上。
- The blue arrow shows requests only; MCP responses are omitted for clarity. 蓝色箭头只展示请求方向，省略 MCP 响应。
- The MCP mark references the [official asset](https://github.com/modelcontextprotocol/modelcontextprotocol/blob/main/docs/mcp.png). MCP 标志参考官方素材。
- The cat avatar is Anan's existing brand artwork; the phone/StopWatch illustration follows the user's supplied style reference. 猫咪使用 Anan 现有品牌头像，设备插画沿用用户提供的风格参考。

## English localization prompt / 英文版生成提示词

The approved Chinese illustration was the edit input. 以已确认的中文版为编辑输入。

```text
Use case: text-localization. Create an ENGLISH version of this exact iPhone Agent architecture illustration for an English GitHub README. This is a translation-only edit, not a redesign. Keep the original aspect ratio, full canvas, two rounded white cards, 3D sticker iPhone and round StopWatch, cat avatar identity, official MCP symbol, palette, shadows and ALL arrow directions. Replace EVERY Chinese label with the following exact English text, fitting each naturally in its current area with modest font-size adjustments only:
1. Top title 'iphone Agent 操作屏幕' -> 'iPhone Agent Screen Control' (one centered line).
2. Inside iPhone, '屏幕共享' -> 'Screen sharing'. Two short lines 'Screen' and 'sharing' are allowed if needed to fit beside the internal upward arrow; keep clear of the avatar and app grid.
3. Phone bottom '系统与 App' -> 'System & apps'.
4. Gray caption under Wi-Fi '发送请求' -> 'Send requests'.
5. Light blue badge next to the MCP logo '内置 MCP Server' -> 'Built-in MCP Server'. Keep the logo at the left and readable text at the right, INSIDE the StopWatch card, ABOVE the watch. This badge must clearly remain part of the StopWatch device group.
6. Purple subtitle '（模拟蓝牙鼠标键盘）' -> '(Mouse & keyboard emulation)'. Keep 'BLE HID' above it, as two centered lines in violet, readable without touching the purple arrow.
7. Bottom right '接收 Agent 请求' -> 'Receive agent requests'.
8. Bottom right '→ 转换为蓝牙输入' -> '→ Convert to Bluetooth input'.
Keep all already-English labels unchanged: 'iPhone', 'Anan · iOS Agent', 'Wi-Fi · HTTP MCP', 'M5Stack StopWatch', 'ESP32-S3', 'BLE HID'. Crucially preserve upper blue ONE-WAY request arrow FROM Anan/iPhone on LEFT TO StopWatch on RIGHT, lower violet ONE-WAY input arrow FROM StopWatch on RIGHT TO iPhone on LEFT, and internal white ONE-WAY arrow FROM system/apps UP TO Anan. No Chinese text may remain. No extra objects, logos, arrows, servers or computers; no watermark. All English words must be correctly spelled and cleanly rendered.
```
