#!/usr/bin/env bash
# Offline acceptance defined before implementation; no device/radio access.
set -euo pipefail
source "$(dirname "$0")/firmware_paths.sh"
firmware_dir="$(cd "$(dirname "$0")/.." && pwd -P)"
wireless_check_dir="$(mktemp -d "${TMPDIR:-/tmp}/anan-wireless-check.XXXXXX")"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -DINPUT_HOST_CHECK "${firmware_headers[@]}" \
  "$firmware_dir/tools/wireless_acceptance/transport.c" "$firmware_dir/main/input/input.c" "$firmware_dir/main/ui/eyes.c" -lm \
  -o "$wireless_check_dir/transport"
"$wireless_check_dir/transport"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
  "$firmware_dir/tools/wireless_acceptance/reports.c" "$firmware_dir/main/input/ble_reports.c" \
  -o "$wireless_check_dir/reports"
"$wireless_check_dir/reports"
(cd "$firmware_dir" && python3 tools/wireless_acceptance/ble-held.py)
(cd "$firmware_dir" && node tools/wireless_acceptance/scan-page.cjs)

"${CC:-cc}" -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror "${firmware_headers[@]}" "$firmware_dir/tools/wireless_acceptance/network-store.c" "$firmware_dir/main/network/network_store.c" -o "$wireless_check_dir/network-store"
"$wireless_check_dir/network-store"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" "$firmware_dir/tools/wireless_acceptance/authority.c" "$firmware_dir/main/network/network_authority.c" -o "$wireless_check_dir/authority"
"$wireless_check_dir/authority"
(cd "$firmware_dir" && SETUP_AP=1 node tools/wireless_acceptance/scan-page.cjs)

"${CC:-cc}" -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror "${firmware_headers[@]}" "$firmware_dir/tools/wireless_acceptance/wifi-qr.c" "$firmware_dir/main/network/wifi_qr.c" "$firmware_dir/main/vendor/qrcodegen.c" -o "$wireless_check_dir/wifi-qr"
"$wireless_check_dir/wifi-qr" "$wireless_check_dir/synthetic-wifi-qr.pgm"
python3 "$firmware_dir/tools/wireless_acceptance/startup.py"
python3 "$firmware_dir/tools/wireless_acceptance/tools-response.py"
python3 "$firmware_dir/tools/wireless_acceptance/memory.py"
python3 "$firmware_dir/tools/wireless_acceptance/reset-trace.py"
