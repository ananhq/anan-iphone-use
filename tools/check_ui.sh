#!/usr/bin/env bash
# Host-only checks defined before UI implementation; no device actions.
set -euo pipefail
source "$(dirname "$0")/firmware_paths.sh"
ui_root="$(cd "$(dirname "$0")/.." && pwd -P)"
ui_check_dir="$(mktemp -d "${TMPDIR:-/tmp}/anan-ui-check.XXXXXX")"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/layout.c" "$ui_root/main/ui/ui_layout.c" -o "$ui_check_dir/layout"
"$ui_check_dir/layout"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/battery.c" "$ui_root/main/hardware/battery.c" -o "$ui_check_dir/battery"
"$ui_check_dir/battery"
python3 "$ui_root/tools/ui_acceptance/device.py"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/eyes.c" "$ui_root/main/ui/eyes.c" -lm -o "$ui_check_dir/eyes"
"$ui_check_dir/eyes"
python3 "$ui_root/tools/ui_acceptance/eyes-render.py"
python3 "$ui_root/tools/ui_acceptance/eyes-worker.py"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/eyes-visibility.c" "$ui_root/main/ui/eyes.c" -lm -o "$ui_check_dir/eyes-visibility"
"$ui_check_dir/eyes-visibility"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/tilt.c" "$ui_root/main/hardware/tilt.c" -lm -o "$ui_check_dir/tilt"
"$ui_check_dir/tilt"
bash "$ui_root/tools/check_upstream.sh"
bash "$ui_root/tools/check_activity.sh"
bash "$ui_root/tools/check_responsive.sh"
python3 "$ui_root/tools/ui_acceptance/display-region.py"
python3 "$ui_root/tools/ui_acceptance/display-compact.py"
python3 "$ui_root/tools/ui_acceptance/imu.py"
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" \
    "$ui_root/tools/ui_acceptance/avatar-timing.c" "$ui_root/main/ui/avatar_timing.c" -o "$ui_check_dir/avatar-timing"
"$ui_check_dir/avatar-timing"
