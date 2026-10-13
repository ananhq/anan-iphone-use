# Setup, flashing and recovery

English · [简体中文](setup.zh-CN.md)

## Build once on a computer

Use the StopWatch native Type-C port and a data cable. Install official ESP-IDF **v5.5.5** recursively, run `install.sh esp32s3`, source `export.sh`, and ensure CMake ≥3.22/Ninja. Other IDF/Arduino versions are not this build. The first build retrieves esp_tinyusb 2.2.0 / TinyUSB 0.21.0~2 using the checked-in lock.

```sh
./build.sh
shasum -a 256 build/anan-stopwatch-flash.bin
```

Compare the hash against `build/anan-stopwatch-manifest.json`; this records sources, dependency-lock hash, image size and validation flags. The merged image includes bootloader, partition table and application. Flash target is ESP32-S3, **16MB**, merged offset **0x0**. Do not write the merged image at the application offset. No Release binary is published; build locally. Historical and migrated hashes are recorded separately in [validation](validation.md).

## Flash only after checking the target

Follow the [official download-mode instructions](https://docs.m5stack.com/en/core/StopWatch): connect to a computer, hold the power button about 2 seconds until the green LED lights, then release. Identify the actual port by connecting/disconnecting; do not copy the placeholder.

```sh
./flash.sh /dev/cu.usbmodemXXXX
# If already in download mode and normal reset cannot connect:
python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX \
  --baud 460800 --before no_reset write_flash --flash_size 16MB \
  0x0 build/anan-stopwatch-flash.bin
```

Require a successful write/hash verification. Short-press power to reset. Black AMOLED is expected: no display driver is initialized. Flashing replaces factory firmware; restore the correct StopWatch factory package via official M5Burner and its instructions. These scripts do not modify eFuses or enable Secure Boot. This migration does not flash any device.

## Configure the phone

| Item | Value/action |
| --- | --- |
| Cable | StopWatch Type-C → unlocked portrait USB-C iPhone |
| IPv4 | USB Ethernet manual 192.168.7.2, mask 255.255.255.0; router empty, no DNS |
| Pointer | Enable Settings → Accessibility → Touch → AssistiveTouch |
| Keyboard | Hardware layout US; TYPE supports printable ASCII, TAB/LF |
| Client | Allow Anan local-network access; connect StopWatch, start whole-display sharing, check local preview |
| Agent | Configure an image-capable model; enable control for the next task |

Anan's capture requires iOS 27+. The USB subnet is not an internet gateway; retain the phone's existing Wi-Fi/cellular model connection. HID/Ethernet does not itself require developer mode. Current Anan developer-signed installation does; TestFlight/App Store distribution is not validated by this migration.

Start with observe-only, then one small input in an empty test note; observe after every action. Each new Anan task requires new control permission. Stop via either front button, Anan stop or cable removal. Reconnect explicitly and inspect partial effects; never automatically replay a failed command.

## Troubleshooting

| Symptom | Next check |
| --- | --- |
| No Ethernet | Data cable, exit bootloader/reset, correct 16MB image |
| No READY/timeout | Manual IP/mask, local-network permission, stop button released |
| Preview invalid | Unlock/portrait, whole-display consent; protected content may be unavailable |
| Pointer/click mismatch | AssistiveTouch and overlay; stop and inspect coordinates |
| Wrong characters | US hardware layout; no Unicode/IME support |
| Background stalled | Inspect the phone client's capture/execution state; firmware does not keep iOS alive |
| Partial text/unknown result | Observe manually, reconnect/re-authorize; no replay |

Use synthetic screens/notes and keep private screenshots/device identifiers out of logs. A future device run needs separately approved scope and a record of firmware hash, client version, OS and observed outcomes.
