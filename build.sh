#!/usr/bin/env bash
set -euo pipefail
firmware_dir="$(cd "$(dirname "$0")" && pwd -P)"
: "${IDF_PATH:?Source ESP-IDF v5.5.5 export.sh before building.}"
if [[ "$(idf.py --version)" != "ESP-IDF v5.5.5" ]]; then
  echo 'Expected ESP-IDF v5.5.5; use the pinned toolchain.' >&2
  exit 1
fi
idf.py -C "$firmware_dir" build
python "$firmware_dir/package.py" "$firmware_dir/build"
