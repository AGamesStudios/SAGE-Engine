from __future__ import annotations

import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "native" / "nano_core"
FLAG = ROOT / "reload.flag"


def build() -> None:
    manifest = SRC / "Cargo.toml"
    subprocess.run([
        "maturin",
        "develop",
        "--release",
        "--manifest-path",
        str(manifest),
    ], check=True)
    FLAG.touch()


def latest_mtime() -> float:
    return max(p.stat().st_mtime for p in SRC.rglob("*.rs"))


def watch() -> None:
    last = latest_mtime()
    while True:
        time.sleep(1.0)
        cur = latest_mtime()
        if cur != last:
            build()
            last = cur


if __name__ == "__main__":
    watch()
