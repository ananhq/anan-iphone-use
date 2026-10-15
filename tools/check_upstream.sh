#!/usr/bin/env bash
# Predefined host-only port acceptance, no device I/O.
set -euo pipefail
source "$(dirname "$0")/firmware_paths.sh"
root="$(cd "$(dirname "$0")/.." && pwd -P)"
out="$(mktemp -d "${TMPDIR:-/tmp}/anan-avatar-upstream.XXXXXX")"
cc -std=c11 -Wall -Wextra -Werror "${firmware_headers[@]}" -c "$root/main/ui/eyes.c" -o "$out/eyes.o"
c++ -std=c++17 -Wall -Wextra -Werror "${firmware_headers[@]}" \
  "$root/tools/ui_acceptance/upstream.cpp" "$root/main/ui/avatar.cpp" "$root/main/ui/avatar_native.cpp" "$root/main/ui/avatar_activity.cpp" \
  "$root/main/vendor/stopwatch_avatar/avatar_engine.cpp" "$out/eyes.o" -o "$out/check"
"$out/check"
