#!/usr/bin/env python3
"""Predefined offline flash acceptance; never loads esptool or opens a device."""
import importlib.util
import pathlib
import struct
import subprocess
import tempfile
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
LAYOUT = ((1, 2, 0x9000, 0x6000, b"nvs"),
          (1, 1, 0xf000, 0x1000, b"phy_init"),
          (0, 0, 0x10000, 0x300000, b"factory"),
          (1, 2, 0xf00000, 0x10000, b"mcp_kv"))


def image():
    data = bytearray(b"\xff" * 0x11031)
    data[0] = data[0x10000] = 0xe9
    struct.pack_into("<H", data, 12, 9)
    for index, (kind, subtype, offset, size, label) in enumerate(LAYOUT):
        start = 0x8000 + index * 32
        data[start:start + 32] = struct.pack("<HBBII16sI", 0x50aa, kind,
                                           subtype, offset, size, label, 0)
    data[0xf000:] = bytes((i % 251 for i in range(len(data) - 0xf000)))
    data[0x10000] = 0xe9
    struct.pack_into("<H", data, 0x1000c, 9)
    return bytes(data)


def main():
    spec = importlib.util.spec_from_file_location("flash_helper", ROOT / "flash.py")
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    source = image()
    with tempfile.TemporaryDirectory(prefix="stopwatch flash acceptance ") as tmp:
        path = pathlib.Path(tmp) / "merged image.bin"
        path.write_bytes(source)
        flash = bytearray(b"\xff" * 0x1000000)
        calls = []

        def write(command, **kwargs):
            calls.append(command)
            assert command[:3] == [helper.sys.executable, "-m", "esptool"]
            assert command[command.index("--port") + 1] == "/dev/synthetic"
            assert command[command.index("--before") + 1] == "no_reset"
            items = command[command.index("16MB") + 1:]
            assert len(items) == 4
            assert [int(items[0], 0), int(items[2], 0)] == [0, 0xf000]
            for offset, name in zip(items[::2], items[1::2]):
                start = int(offset, 0)
                payload = pathlib.Path(name).read_bytes()
                end = (start + len(payload) + 0xfff) & ~0xfff
                assert not (start < 0xf000 and end > 0x9000)
                assert end <= 0xf00000
                flash[start:end] = b"\xff" * (end - start)
                flash[start:start + len(payload)] = payload
            return subprocess.CompletedProcess(command, 0)

        with mock.patch.object(helper.subprocess, "run", side_effect=write):
            assert helper.main(["/dev/synthetic", str(path), "--before", "no_reset"]) == 0
            assert flash[0x9000:0xf000] == b"\xff" * 0x6000
            flash[0x9000:0xf000] = b"B" * 0x6000
            flash[0xf00000:0xf10000] = b"M" * 0x10000
            for _ in range(2):
                assert helper.main(["/dev/synthetic", str(path), "--before", "no_reset"]) == 0
        assert len(calls) == 3
        assert flash[:0x9000] == source[:0x9000]
        assert flash[0xf000:len(source)] == source[0xf000:]
        assert flash[0x9000:0xf000] == b"B" * 0x6000
        assert flash[0xf00000:0xf10000] == b"M" * 0x10000

        invalid = [source[:0x10000], b"\x00" + source[1:], source + b"\xff" * 0xf00000]
        for offset in (12, 0x8004, 0x8020, 0x8064, 0x9000, 0x10000, 0x1000c):
            data = bytearray(source)
            data[offset] ^= 1
            invalid.append(bytes(data))
        with mock.patch.object(helper.subprocess, "run") as run:
            for data in invalid:
                path.write_bytes(data)
                assert helper.main(["/dev/synthetic", str(path)]) != 0
            assert not run.called
        path.write_bytes(source)
        with mock.patch.object(helper.subprocess, "run", side_effect=subprocess.CalledProcessError(7, [])):
            assert helper.main(["/dev/synthetic", str(path)]) == 7
        assert not any(pathlib.Path(name).exists() for name in calls[0][-3::2])
    print("PASS: offline flash ranges, NVS retention, invalid-image rejection and failure propagation")


if __name__ == "__main__":
    main()
