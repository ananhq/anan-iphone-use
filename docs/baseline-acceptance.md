# Baseline acceptance

English · [简体中文](baseline-acceptance.zh-CN.md)

Status: preparation and compilation/static checks are complete, and the user
reported a successful reflash. They then explicitly authorized a first local
checkpoint while input/stop checks and source gaps remain pending. That
checkpoint does not establish full baseline acceptance. This procedure does
not authorize an assistant to flash or operate a device.

## Identify the candidate

Build with ESP-IDF v5.5.5 and `./build.sh`. Check
`build/anan-stopwatch-manifest.json` and compute:

```sh
shasum -a 256 build/anan-stopwatch-flash.bin
```

The candidate must be M5Stack StopWatch / ESP32-S3 / 16MB, with merged offset
`0x0`. Use [setup and recovery](setup.md) to flash the actual board. Record the
image hash and successful write/hash verification; omit the actual serial port,
device MAC, other device identifiers and raw logs from shareable evidence.

The manifest's `flashed` and `deviceVerified` flags start false because packaging
cannot establish device execution. A rebuild creates a new manifest and does
not carry prior acceptance forward. Record manual results separately against
the exact image hash, with the date and relevant board/OS/client versions.

## Proposed manual scope

Use an unlocked portrait iPhone, the existing [Anan](https://anan.sh) client, USB-C data cable,
manual USB IPv4 and AssistiveTouch. Use an empty synthetic test note. The user
can report pass, fail, not run or outcome unknown for each row. These scenarios
are a proposal, not a claim that they have been executed or accepted.

| Check | Observable result |
| --- | --- |
| Flash and reset | Write/hash verification succeeds; board starts; black display is expected |
| USB and connection | USB Ethernet and input devices are recognized; Anan remains connected for at least 60 seconds |
| Preview and observation | Full portrait screen is visible; observe returns the actual screen without causing input |
| Tap | One chosen target is visibly activated, without duplicate input |
| ASCII/key | A short ASCII string appears exactly once; one Return press has the expected visible effect |
| Drag | One chosen scroll/drag has the expected direction and result |
| Anan stop | Interrupt a bounded input; further input stops and any partial effect is recorded |
| Front buttons | Test each button separately during input; further input stops |
| Detach | Unplug during input; reconnect only explicitly; no queued/old action resumes |
| Task lifecycle | End a task; another task needs new permission; an unknown result is not automatically replayed |

The 60-second connection check is a manual observation, not proof of packet-level
READY/PONG coverage or a measured stop-latency guarantee. Record visible partial
effects; stop does not undo input already delivered. Re-enable only explicitly
between checks. Do not test on private or consequential app content.

No-Mac multi-step tasks and extended background endurance are separate checks;
if not run, keep them unverified. Do not describe a narrow input check as full
task/endurance acceptance. Real model calls are chosen by the user for their
manual run, not initiated by repository preparation.

## Report and commit

Report the date, candidate SHA-256, relevant versions, results by row and any
failures or unknown effects. A short text report is sufficient; private screens
and raw logs are not required. Device results and provenance are separate:
flashing successfully does not resolve [source gaps](../THIRD_PARTY.md).

After the user confirms the actual acceptance scope and source handling, add a
sanitized validation record, update both validation documents, recheck that the
candidate's build inputs are unchanged and create the local baseline commit.
Record the accepted baseline commit ID when that stage completes. The earlier
local checkpoint is identified by Git history and the
[checkpoint record](../artifacts/baseline/checkpoint.json). Keep failed or unrun
checks explicit. A local commit does not authorize a push or public release.

This procedure records the historical ANAN1 checkpoint scope. Current
`0.4.0-mcp` retires that service; use [MCP setup](mcp.md) and current
[validation](validation.md) for the new candidate, not READY/PONG acceptance.
