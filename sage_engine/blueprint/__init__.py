"""Blueprint schema and validation utilities."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Mapping
import json

from ..logger import logger
from .. import compat



@dataclass
class BlueprintMeta:
    origin: str | None = None
    tags: List[str] = field(default_factory=list)


@dataclass
class Blueprint:
    meta: BlueprintMeta
    objects: List[Mapping[str, Mapping[str, object]]]


def load(path: Path) -> Blueprint:
    """Load and validate a blueprint from JSON."""
    data = json.loads(path.read_text(encoding="utf8"))
    meta = data.get("meta", {})
    bp_meta = BlueprintMeta(
        origin=meta.get("origin"),
        tags=list(meta.get("tags", [])),
    )
    objects = []
    for obj in data.get("objects", []):
        compat.migrate(obj, "blueprint_object")
        objects.append(obj)
    if not isinstance(objects, list):
        raise ValueError("objects must be a list")
    return Blueprint(bp_meta, objects)


__all__ = ["Blueprint", "BlueprintMeta", "load"]


from .. import core

core.expose(
    "blueprint",
    {
        "Blueprint": Blueprint,
        "BlueprintMeta": BlueprintMeta,
        "load": load,
    },
)
