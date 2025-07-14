#!/usr/bin/env python3
"""Compile the nano_core Rust extension using maturin."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent


def build():
    manifest = ROOT / "native" / "nano_core" / "Cargo.toml"
    subprocess.run([
        "maturin",
        "develop",
        "--release",
        "--manifest-path",
        str(manifest),
    ], check=True)


if __name__ == "__main__":
    build()
