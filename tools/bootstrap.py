"""Setup development environment and run tests."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def check_rustup() -> None:
    if not shutil.which("rustup"):
        raise SystemExit("rustup not found")


def install_maturin() -> None:
    subprocess.run([sys.executable, "-m", "pip", "install", "--upgrade", "maturin"], check=True)


def build_nano_core() -> None:
    manifest = ROOT / "native" / "nano_core" / "Cargo.toml"
    subprocess.run([
        "maturin",
        "develop",
        "--release",
        "--manifest-path",
        str(manifest),
    ], check=True)


def run_tests() -> None:
    subprocess.run(["ruff", "check", "."], check=True)
    env = dict(os.environ, PYTHONPATH="src")
    subprocess.run([sys.executable, "-m", "pytest", "-q"], check=True, env=env)


if __name__ == "__main__":
    check_rustup()
    install_maturin()
    build_nano_core()
    run_tests()
