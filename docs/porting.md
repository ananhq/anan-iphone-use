# Porting guide and prompt

English · [简体中文](porting.zh-CN.md)

StopWatch is the only current target. Not every ESP32 has configurable native USB Device support. A USB-UART programming connector is not necessarily the native USB route. Do not flash the StopWatch binary onto another board.

Before editing, collect exact chip/board/revision, schematic, USB D+/D− and power routing, Flash/PSRAM, bootloader procedure, power controller/registers, buttons/active levels, screen/touch drivers and documented licenses. Verify each against official sources. Separate portable TCP/parser/HID logic from board power/stop mapping; migration itself adds no board abstraction.

Keep the source checkpoint and ANAN1 behavior first. Propose board changes before implementation, then compile/package and record hashes. A compile result cannot establish USB enumeration, correct stop behavior or phone coordinates; define synthetic device scenarios and obtain approval before flashing/inputs. Adapt screens only after the baseline and MCP phases, without starving heartbeat/stop/report release. Retain upstream notices and identify reused files.

## Copy-ready prompt

```text
Adapt this repository to my board. Do research first; do not flash or send inputs.
Board/chip/revision: <exact values>
Official documentation and schematic: <links>
Native USB connector and D+/D− pins: <values, evidence>
Power supply/controller/registers: <values, evidence>
Flash/PSRAM and boot/download procedure: <values>
Stop buttons, GPIOs and active levels: <values>
Screen/touch controller/pins: <values or unknown>
Phone/client and expected connection: <values>

Read AGENTS.md, docs/design.md, PROTOCOL.md, THIRD_PARTY.md and the source.
List confirmed facts and unknowns with official source references. Explain which
files must change, dependency/licenses and stop/power risks. Present a minimal
plan and wait for confirmation before coding. Keep ANAN1/HID behavior unless a
protocol change is expressly agreed. Preserve provenance; never conceal reuse.
Compile/package without accessing a device, record source/image/tool versions,
and mark runtime checks pending. Define device acceptance scenarios before code;
request a separate approved scope before flashing, E2E, model calls or input.
Do not implement MCP or a display UI as an incidental board-port change.
```
