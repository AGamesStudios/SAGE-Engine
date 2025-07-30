from __future__ import annotations

"""Helpers for simple draw call batching."""

from collections import defaultdict
from typing import Iterable, List, Tuple, Dict


Command = Tuple[int, int, int, int, object]


def batch_rect(commands: Iterable[Command]) -> Dict[Tuple[int, int, object], List[Tuple[int, int]]]:
    """Group draw_rect commands by size and color."""
    batches: Dict[Tuple[int, int, object], List[Tuple[int, int]]] = defaultdict(list)
    for x, y, w, h, color in commands:
        batches[(w, h, color)].append((x, y))
    return batches
