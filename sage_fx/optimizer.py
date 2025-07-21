"""Simple optimizer for FX operations."""
from __future__ import annotations

from typing import Iterable, List

from .parser import Operation


def optimize_ops(ops: Iterable[Operation]) -> List[Operation]:
    """Remove redundant and noop operations."""
    optimized: List[Operation] = []
    prev: Operation | None = None
    for op in ops:
        if op.name == "noop":
            continue
        if prev and op == prev:
            continue
        optimized.append(op)
        prev = op
    return optimized
