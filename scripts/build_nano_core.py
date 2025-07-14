#!/usr/bin/env python3
"""Compile the nano_core Rust extension using maturin."""
from pathlib import Path
import subprocess
import os
import sysconfig
import shutil

ROOT = Path(__file__).resolve().parent.parent


def _compress(path: Path) -> None:
    if not shutil.which("upx"):
        return
    subprocess.run(["upx", "-q", str(path)], check=True)


def build() -> None:
    manifest = ROOT / "native" / "nano_core" / "Cargo.toml"
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


if __name__ == "__main__":
    build()

