#!/usr/bin/env bash
# Offline scenarios authored before implementation; no USB/device/provider access.
set -euo pipefail
firmware_dir="$(cd "$(dirname "$0")/.." && pwd -P)"
: "${IDF_PATH:?Source the pinned ESP-IDF v5.5.5 environment first.}"
hid_check_dir="$(mktemp -d "${TMPDIR:-/tmp}/anan-hid-check.XXXXXX")"
"${CC:-cc}" -std=c11 -Wall -Wextra -Wno-unused-function \
  -I"$firmware_dir/tools/hid_acceptance" -I"$firmware_dir/main" \
  -I"$IDF_PATH/components/json/cJSON" \
  "$firmware_dir/tools/hid_acceptance/acceptance.c" "$firmware_dir/main/hid_input.c" \
  "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o "$hid_check_dir/acceptance"
"$hid_check_dir/acceptance"
