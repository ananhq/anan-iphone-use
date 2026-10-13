#!/usr/bin/env python3
"""Merge an already compiled IDF build. Never opens a serial port."""
import hashlib
import json
import pathlib
import subprocess
import sys
from datetime import datetime, timezone

build = pathlib.Path(sys.argv[1]).resolve()
args = json.loads((build / "flasher_args.json").read_text())
assert args["extra_esptool_args"]["chip"] == "esp32s3"
settings = args["flash_settings"]
assert settings["flash_size"] == "16MB"
output = build / "anan-stopwatch-flash.bin"
command = [sys.executable, "-m", "esptool", "--chip", "esp32s3", "merge_bin",
           "-o", str(output), "--flash_mode", settings["flash_mode"],
           "--flash_freq", settings["flash_freq"], "--flash_size", "16MB"]
files = sorted(args["flash_files"].items(), key=lambda item: int(item[0], 0))
for address, name in files:
    command.extend([address, str(build / name)])
subprocess.run(command, check=True)
# Credential storage is never packaged; ordinary writes must stop before mcp_kv.
assert output.stat().st_size <= 0xf00000
source = pathlib.Path(__file__).parent
inputs = [path for path in (source / "main").rglob("*") if path.is_file()]
inputs += [source / name for name in ("CMakeLists.txt", "sdkconfig.defaults", "dependencies.lock", "package.py", "build.sh", "flash.sh", "partitions.csv")]
source_hashes = {str(path.relative_to(source)): hashlib.sha256(path.read_bytes()).hexdigest()
                 for path in sorted(inputs)}
manifest = {"protocol": "MCP", "mcp": {"version": "2025-11-25", "transport": "HTTP/HTTPS Streamable HTTP"}, "board": "M5Stack StopWatch ESP32-S3 16MB",
            "idf": "5.5.5", "createdAt": datetime.now(timezone.utc).isoformat(),
            "binary": output.name, "flashOffset": "0x0", "flashSize": "16MB",
            "sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
            "bytes": output.stat().st_size, "sources": source_hashes,
            "idfVersion": subprocess.check_output(["idf.py", "--version"], text=True).strip(),
            "componentLockSHA256": source_hashes["dependencies.lock"],
            "validation": {"compiled": True, "flashed": False, "deviceVerified": False}}
(build / "anan-stopwatch-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
print(f"Flash image: {output}\nSHA-256: {manifest['sha256']}")
