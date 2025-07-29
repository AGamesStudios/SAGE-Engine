"""Flow testing helpers."""
from __future__ import annotations

from typing import Iterable


def assert_path_passed(flow: Iterable[str], path: Iterable[str]) -> None:
    seq = list(flow)
    for step in path:
        if step not in seq:
            raise AssertionError(f"step {step!r} not passed")
