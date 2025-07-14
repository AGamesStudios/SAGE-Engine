from __future__ import annotations

import subprocess
import time
import os
from pathlib import Path
import sysconfig
import shutil
import asyncio
import websockets

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "native" / "nano_core"
FLAG = ROOT / "reload.flag"
WS_URL = os.environ.get("SAGE_RELOAD_WS", "ws://localhost:8765")


def _compress(path: Path) -> None:
    if not shutil.which("upx"):
        return
    subprocess.run(["upx", "-q", str(path)], check=True)


def build() -> None:
    manifest = SRC / "Cargo.toml"
    build_type = os.environ.get("SAGE_BUILD", "release").lower()
    args = [
        "maturin",
        "develop",
        "--manifest-path",
        str(manifest),
    ]
    if build_type == "release":
        args.insert(2, "--release")
    subprocess.run(args, check=True)
    if build_type == "release":
        platlib = Path(sysconfig.get_paths()["platlib"])
        for so in platlib.glob("nano_core*.so"):
            _compress(so)
    FLAG.touch()
    asyncio.run(_notify())


def latest_mtime() -> float:
    return max(p.stat().st_mtime for p in SRC.rglob("*.rs"))


async def _notify() -> None:
    try:
        async with websockets.connect(WS_URL) as ws:
            await ws.send("reload nano_core")
    except Exception:
        pass


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

