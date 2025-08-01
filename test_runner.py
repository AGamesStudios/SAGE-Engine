#!/usr/bin/env python3
"""Run SAGE Engine tests with resource verification."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main(argv: list[str] | None = None) -> int:
    repo = Path(__file__).resolve().parent
    font = repo / "sage_engine" / "resources" / "fonts" / "default.ttf"
    if not font.exists():
        print("missing resources/fonts/default.ttf", file=sys.stderr)
        return 1
    cmd = [sys.executable, "-m", "pytest", "-q"]
    if argv:
        cmd.extend(argv)
    return subprocess.call(cmd)


if __name__ == "__main__":
    raise SystemExit(main())
