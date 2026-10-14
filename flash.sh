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
"${PYTHON:-python}" "$firmware_dir/flash.py" "$serial_port" "$flash_image"
