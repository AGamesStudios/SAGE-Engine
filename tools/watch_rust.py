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
ERROR_LOG = ROOT / "watch_errors.log"


def _compress(path: Path) -> None:
    if not shutil.which("upx"):
        return
    subprocess.run(["upx", "-q", str(path)], check=True)


def build() -> bool:
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
    proc = subprocess.run(args, capture_output=True, text=True)
    if proc.returncode != 0:
        with open(ERROR_LOG, "a", encoding="utf-8") as f:
            f.write(proc.stdout)
            f.write(proc.stderr)
        asyncio.run(_toast("Rust build failed; see watch_errors.log"))
        return False
    if build_type == "release":
        platlib = Path(sysconfig.get_paths()["platlib"])
        for so in platlib.glob("nano_core*.so"):
            _compress(so)
    FLAG.touch()
    asyncio.run(_notify())
    return True


def latest_mtime() -> float:
    return max(p.stat().st_mtime for p in SRC.rglob("*.rs"))


async def _notify() -> None:
    try:
        async with websockets.connect(WS_URL) as ws:
            await ws.send("reload nano_core")
    except Exception:
        pass


async def _toast(message: str) -> None:
    try:
        async with websockets.connect(WS_URL) as ws:
            await ws.send(f"toast {message}")
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

