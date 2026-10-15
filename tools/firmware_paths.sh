#!/usr/bin/env bash
# Shared header search paths for host-only checks; source from Bash entrypoints.
firmware_headers_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
firmware_headers=()
for firmware_headers_group in "" app hardware input network ui; do
    firmware_headers+=(-I"$firmware_headers_root/main${firmware_headers_group:+/$firmware_headers_group}")
done
unset firmware_headers_root firmware_headers_group
