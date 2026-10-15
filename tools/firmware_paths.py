"""Header search paths for host-only firmware checks and previews."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER_DIRS = ("", "app", "hardware", "input", "network", "ui")


def include_flags():
    return [flag for name in HEADER_DIRS
            for flag in ("-I", str(ROOT / "main" / name))]
