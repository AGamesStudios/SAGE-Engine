"""Compatibility and data migration helpers."""

from __future__ import annotations

from typing import Callable, Dict, Dict as DictType, Tuple

from ..logger import logger


_registry: DictType[str, DictType[int, Tuple[int, Callable[[dict], dict]]]] = {}


def register(schema_type: str, from_version: int, to_version: int, func: Callable[[dict], dict]) -> None:
    """Register a migration *func* for *schema_type* from *from_version* to *to_version*."""
    _registry.setdefault(schema_type, {})[from_version] = (to_version, func)


def migrate(schema_type: str, version: int, target: int, data: dict) -> tuple[int, dict]:
    """Migrate *data* from *version* to *target* applying registered migrations."""
    original = version
    while version != target:
        entry = _registry.get(schema_type, {}).get(version)
        if entry is None:
            break
        to, func = entry
        logger.info("Migrated %s from %s -> %s", schema_type, version, to)
        data = func(data)
        version = to
    if original != version:
        logger.debug("%s fully migrated from %s to %s", schema_type, original, version)
    return version, data

__all__ = ["register", "migrate", "migrate_schema"]


def migrate_schema(data: dict, current_version: int, latest_version: int, schema_type: str, /) -> dict:
    """Migrate *data* using registered migrations up to *latest_version*."""
    version = current_version
    while version < latest_version:
        entry = _registry.get(schema_type, {}).get(version)
        if entry is None:
            break
        to, func = entry
        data = func(data)
        version = to
    data["schema_version"] = version
    return data


def _flow_v1_to_v2(data: dict) -> dict:
    new = dict(data)
    vars_ = new.get("variables", {})
    if "hp" in vars_:
        vars_["health"] = vars_.pop("hp")
    new["variables"] = vars_
    new["schema_version"] = 2
    return new


def _scene_v1_to_v2(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new["schema_version"] = new.pop("engine_version")
    if "entities" in new:
        new["objects"] = new.pop("entities")
    return new


register("flowscript", 1, 2, _flow_v1_to_v2)
register("scene", 1, 2, _scene_v1_to_v2)


def _cfg_v1_to_v2(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new.pop("engine_version")
    new["schema_version"] = 2
    return new

register("engine_cfg", 1, 2, _cfg_v1_to_v2)
