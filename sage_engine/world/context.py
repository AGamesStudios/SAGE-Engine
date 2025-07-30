from __future__ import annotations

"""Data structures for SAGE World."""

from dataclasses import dataclass, field
from typing import List, Dict


@dataclass
class WorldObject:
    role: str
    x: float = 0.0
    y: float = 0.0
    fields: Dict[str, object] = field(default_factory=dict)


@dataclass
class WorldConfig:
    name: str = "world"
    objects: List[WorldObject] = field(default_factory=list)
    scripts: List[str] = field(default_factory=list)
