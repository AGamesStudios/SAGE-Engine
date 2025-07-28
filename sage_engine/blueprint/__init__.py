"""Blueprint schema and validation utilities."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Mapping
import json


@dataclass
class BlueprintMeta:
    version: str
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
    if "version" not in meta:
        raise ValueError("blueprint missing meta.version")
    bp_meta = BlueprintMeta(
        version=str(meta["version"]),
        origin=meta.get("origin"),
        tags=list(meta.get("tags", [])),
    )
    objects = data.get("objects", [])
    if not isinstance(objects, list):
        raise ValueError("objects must be a list")
    return Blueprint(bp_meta, objects)


__all__ = ["Blueprint", "BlueprintMeta", "load"]
