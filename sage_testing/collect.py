"""Test collection helpers."""
from __future__ import annotations

from pathlib import Path
from typing import List


def collect_tests(paths: list[str]) -> List[str]:
    tests: list[str] = []
    for p in paths:
        path = Path(p)
        if path.is_dir():
            tests.append(str(path))
        elif path.is_file():
            tests.append(str(path))
    return tests
