#!/usr/bin/env python3
"""Flash a StopWatch merged image without erasing either NVS partition."""
import argparse
import hashlib
import pathlib
import struct
import subprocess
import sys
import tempfile

LAYOUT = ((1, 2, 0x9000, 0x6000, b"nvs"),
          (1, 1, 0xf000, 0x1000, b"phy_init"),
          (0, 0, 0x10000, 0x300000, b"factory"),
          (1, 2, 0xf00000, 0x10000, b"mcp_kv"))


def validate(data):
    if not 0x10018 <= len(data) <= 0x310000:
        raise ValueError("Image is truncated or exceeds the StopWatch application partition")
    for offset in (0, 0x10000):
        if data[offset] != 0xe9 or struct.unpack_from("<H", data, offset + 12)[0] != 9:
            raise ValueError("Expected ESP32-S3 bootloader and application in a merged image")
    table = data[0x8000:0x9000]
    for index, (kind, subtype, offset, size, label) in enumerate(LAYOUT):
        expected = struct.pack("<HBBII16sI", 0x50aa, kind, subtype, offset, size, label, 0)
        if table[index * 32:(index + 1) * 32] != expected:
            raise ValueError("Unsupported partition layout; use this board's approved flash procedure")
    end = len(LAYOUT) * 32
    if table[end:end + 16] == b"\xeb\xeb" + b"\xff" * 14:
        if table[end + 16:end + 32] != hashlib.md5(table[:end]).digest():
            raise ValueError("Invalid partition-table checksum")
        end += 32
    if table[end:] != b"\xff" * (len(table) - end):
        raise ValueError("Unexpected partition-table entries or signature")
    if data[0x9000:0xf000] != b"\xff" * 0x6000:
        raise ValueError("Image includes NVS data; refusing to discard or write pairing records")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="Explicit StopWatch serial port")
    parser.add_argument("image", nargs="?", type=pathlib.Path,
                        default=pathlib.Path(__file__).resolve().parent / "build/anan-stopwatch-flash.bin")
    parser.add_argument("--before", choices=("default_reset", "no_reset"), default="default_reset")
    args = parser.parse_args(argv)
    try:
        data = args.image.read_bytes()
        validate(data)
        # Both starts and the first length align to 4 KiB erase sectors. The
        # application tail's final erase sector cannot reach either NVS region.
        with tempfile.TemporaryDirectory(prefix="stopwatch-flash-") as tmp:
            boot = pathlib.Path(tmp) / "boot-and-partitions.bin"
            app = pathlib.Path(tmp) / "phy-and-application.bin"
            boot.write_bytes(data[:0x9000])
            app.write_bytes(data[0xf000:])
            print("Writing StopWatch firmware; preserving BLE NVS and MCP/Wi-Fi settings.", flush=True)
            subprocess.run([sys.executable, "-m", "esptool", "--chip", "esp32s3",
                            "--port", args.port, "--baud", "460800", "--before", args.before,
                            "write_flash", "--compress", "--flash_size", "16MB",
                            "0x0", str(boot), "0xf000", str(app)], check=True)
    except subprocess.CalledProcessError as error:
        return error.returncode
    except (OSError, ValueError) as error:
        print(f"Flash failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
