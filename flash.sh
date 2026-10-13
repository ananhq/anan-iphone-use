#!/usr/bin/env bash
set -euo pipefail
firmware_dir="$(cd "$(dirname "$0")" && pwd -P)"
if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 /dev/cu.usbmodemXXXX [merged-image.bin]" >&2
  exit 1
fi
serial_port="$1"
flash_image="${2:-$firmware_dir/build/anan-stopwatch-flash.bin}"
[[ -f "$flash_image" ]] || { echo "Missing image: $flash_image" >&2; exit 1; }
echo "Writing StopWatch firmware through explicit port: $serial_port"
python -m esptool --chip esp32s3 --port "$serial_port" --baud 460800 write_flash --flash_size 16MB 0x0 "$flash_image"
