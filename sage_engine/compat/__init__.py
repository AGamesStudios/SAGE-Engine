"""Compatibility and data migration helpers."""

from __future__ import annotations

from typing import Callable, Dict, List, Tuple
import logging

logger = logging.getLogger(__name__)

_registry: Dict[str, List[Tuple[str, str, Callable[[dict], dict]]]] = {}


def register(schema_type: str, from_version: str, to_version: str, func: Callable[[dict], dict]) -> None:
    """Register a migration *func* for *schema_type* from *from_version* to *to_version*."""
    _registry.setdefault(schema_type, []).append((from_version, to_version, func))


def migrate(schema_type: str, version: str, target: str, data: dict) -> tuple[str, dict]:
    """Migrate *data* from *version* to *target* applying registered migrations."""
    original = version
    while version != target:
        for frm, to, func in _registry.get(schema_type, []):
            if frm == version:
                logger.info("Migrated %s from %s -> %s", schema_type, frm, to)
                data = func(data)
                version = to
                break
        else:
            break
    if original != version:
        logger.debug("%s fully migrated from %s to %s", schema_type, original, version)
    return version, data

__all__ = ["register", "migrate"]


def _flow_0_9_to_1_0(data: dict) -> dict:
    new = dict(data)
    vars_ = new.get("variables", {})
    if "hp" in vars_:
        vars_["health"] = vars_.pop("hp")
    new["variables"] = vars_
    new["schema_version"] = "1.0"
    return new


def _scene_0_9_to_1_0(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new["schema_version"] = new.pop("engine_version")
    if "entities" in new:
        new["objects"] = new.pop("entities")
    return new


register("flowscript", "0.9", "1.0", _flow_0_9_to_1_0)
register("scene", "0.9", "1.0", _scene_0_9_to_1_0)
