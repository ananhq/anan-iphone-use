# Updates that preserve BLE bonds

English · [简体中文](flashing.zh-CN.md)

## Scope agreed before implementation

The user reports that BLE reconnection fails after reflashing and works after
forgetting the phone's old pairing. NimBLE stores bonds in default `nvs` at
`0x9000–0xefff`. Writing the merged image continuously from `0x0` erases this
region even though the packaged gap contains only padding. Wi-Fi/MCP storage
at `0xf00000` is already outside the image.

Keep the firmware, partition layout, BLE security, dependencies and merged image
format unchanged. The flash helper will validate the image's existing StopWatch
partition layout, split its writes around default `nvs`, and invoke esptool only
after validation. Use the same merged image for first installation and updates.
Retain explicit port selection and support `--before no_reset`. Never read or
export pairing keys, erase the chip, restore control grants or replay input.

## Acceptance defined before implementation

Offline checks will intercept esptool, model its sector erases/writes on synthetic
flash, and verify first installation, repeated updates, exact bytes outside the
skipped region, unchanged synthetic markers in both NVS partitions, unaligned
application tails and manual-reset arguments. Invalid/truncated images, a changed
partition layout, non-padding NVS contents and images reaching `mcp_kv` must fail
before esptool starts. Check paths with spaces, esptool failure propagation,
shell/Python syntax and bilingual setup guidance. No device access is authorized.

Separately authorized device acceptance: pair once, update through the helper,
then reconnect without forgetting the phone's device; check Wi-Fi/MCP settings
and continued explicit input authorization. Offline checks cannot establish
actual iPhone reconnection. Previously erased bonds cannot be recovered.

## Implementation and evidence

`flash.sh` delegates to first-party `flash.py`, using `PYTHON` when supplied.
The helper checks ESP32-S3 image headers, the exact four-partition layout and
optional partition-table MD5, and verifies that the skipped NVS area is padding.
It creates temporary image slices and invokes the selected interpreter's esptool
with writes at `0x0` and `0xf000`; temporary files are removed on success/failure.
The firmware binary and BLE implementation are unchanged. `package.py` now
includes the helper in its source manifest.

Predefined offline scenarios, syntax checks and validation of the existing
`0.7.3-discovery` image passed. Repackaging refreshed source hashes and retained
the same binary hash. See [sanitized record](../artifacts/flashing/check.json).
No firmware recompilation, flash, serial/radio probe, phone action or E2E ran.
