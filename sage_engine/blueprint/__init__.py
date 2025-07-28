"""Blueprint schema and validation utilities."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Mapping
import json

from ..logger import logger

from ..compat import migrate

CURRENT_SCHEMA_VERSION = "1.0"


@dataclass
class BlueprintMeta:
    origin: str | None = None
    tags: List[str] = field(default_factory=list)


@dataclass
class Blueprint:
    schema_version: str
    meta: BlueprintMeta
    objects: List[Mapping[str, Mapping[str, object]]]


def load(path: Path) -> Blueprint:
    """Load and validate a blueprint from JSON."""
    data = json.loads(path.read_text(encoding="utf8"))
    version = str(data.get("schema_version", CURRENT_SCHEMA_VERSION))
    orig_version = version
    version, data = migrate("blueprint", version, CURRENT_SCHEMA_VERSION, data)
    if version != orig_version:
        logger.info("Migrated from %s -> %s", orig_version, version)
    meta = data.get("meta", {})
    bp_meta = BlueprintMeta(
        origin=meta.get("origin"),
        tags=list(meta.get("tags", [])),
    )
    objects = data.get("objects", [])
    if not isinstance(objects, list):
        raise ValueError("objects must be a list")
    return Blueprint(version, bp_meta, objects)


__all__ = ["Blueprint", "BlueprintMeta", "load"]


def _migrate_0_9_to_1_0(data: dict) -> dict:
    new_data = dict(data)
    if "sprite" in new_data:
        new_data["renderable"] = {"sprite": new_data.pop("sprite")}
    new_data["schema_version"] = "1.0"
    return new_data


from ..compat import register as _register

_register("blueprint", "0.9", "1.0", _migrate_0_9_to_1_0)
