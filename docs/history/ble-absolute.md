# Single absolute mouse over BLE

English · [简体中文](ble-absolute.zh-CN.md)

## Agreed comparison and acceptance before implementation

The user reports that `0.7.4-ble-drag` still draws only the first Notes stroke
over BLE while USB works. They approved trying the USB report structure over
BLE before implementing relative-motion calibration. Candidate
`0.7.5-ble-absolute` removes the extra relative mouse collection/report ID4.
Keep absolute mouse ID1, keyboard ID2, Consumer ID3 and keyboard LED output
ID2. The BLE report map must match USB's existing report map exactly.

Coordinates, buttons and wheels go through absolute report ID1. Require the
absolute mouse and keyboard subscriptions; Consumer subscription stays optional.
Use one notification per move/wheel report and release at the last reported
coordinates. Preserve USB, executor pauses, MCP tools/schemas, security, storage,
power, grants and no-replay behavior. No client code, dependencies, calibration
or relative-motion implementation is added.

Before changing production code, redefine the sender fixture to require the
USB-equivalent descriptor list, three input references and the keyboard output
reference, readiness without ID4, combined position/button/wheel payloads,
three independent strokes, held-state release, notification failure without
replay and offline/disconnect behavior. Run existing HID/wireless/flash checks,
pinned SDK compile/package and source/manifest comparisons. Do not add tests
after implementation. Preserve the preceding candidate image and sources.

This changes the BLE report map and GATT table. For separately authorized device
acceptance, forget Anan StopWatch on the phone and reset BLE on the board, then
pair again to remove stale caches/bonds. This is a one-time candidate transition;
normal helper-based firmware updates still preserve NVS. Keep MCP/Wi-Fi settings.
First test an ordinary tap, then three fixed separate strokes in an empty Notes
drawing, plus wheel/key/stop checks. If taps fail, do not treat this candidate as
working or compensate with the model; restore the previous candidate and pair
again. No flash or phone action is authorized by this source/build scope.

BLE acceptance of absolute mouse buttons is the experiment, not an established
fact. Matching USB bytes does not prove matching iOS behavior. The alternative
hybrid relative drag still needs a separately discussed calibration design.

## Candidate results

Predefined host scenarios and the existing HID/wireless/bond-preserving flash
checks passed. The preceding dual-mouse implementation failed the predefined
USB-map equality assertion. Pinned ESP-IDF v5.5.5 compile/package completed with
zero compiler warnings. Both maps extracted from the compiled ELF are exactly
the same 172 bytes. This establishes descriptor equality, not phone acceptance.
Source checks preserve USB, executor timing, seven tools, security/storage and
dependencies. See [sanitized record](../../artifacts/wireless/ble-absolute-build-check.json).

Candidate image: `build/ble-absolute-candidate/anan-stopwatch-flash.bin`.
The previous image and matching source snapshot remain in ignored
`build/ble-drag-candidate/`. Use the current NVS-preserving flash helper, but
perform the explicit BLE reset/re-pair above because the report map changed.
MCP/Wi-Fi setup does not need to be reset. No actual flash or iPhone test ran.
