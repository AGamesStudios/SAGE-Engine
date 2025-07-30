"""Compatibility and data migration helpers."""

from __future__ import annotations

from typing import Callable, Dict, Dict as DictType, Tuple

from ..logger import logger


_registry: DictType[str, DictType[int, Tuple[int, Callable[[dict], dict]]]] = {}
_latest: Dict[str, int] = {}


def register(schema_type: str, func: Callable[[dict], dict], from_version: int | None = None, to_version: int | None = None) -> None:
    """Register a migration function for *schema_type*.

    If *from_version* and *to_version* are omitted, the migration will be
    registered from the latest known version to ``latest + 1``.
    """
    if from_version is None or to_version is None:
        from_version = _latest.get(schema_type, 0)
        to_version = from_version + 1
    _registry.setdefault(schema_type, {})[from_version] = (to_version, func)
    _latest[schema_type] = max(_latest.get(schema_type, 0), to_version)


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


def _scene_v0_to_v1(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new.pop("engine_version")
    new.setdefault("objects", new.pop("entities", []))
    new["schema_version"] = 1
    return new


def _scene_v1_to_v2(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new["schema_version"] = new.pop("engine_version")
    if "entities" in new:
        new["objects"] = new.pop("entities")
    return new


def _cfg_v0_to_v1(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new.pop("engine_version")
    new["schema_version"] = 1
    return new


def _bp_v0_to_v1(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new.pop("engine_version")
    new.setdefault("objects", [])
    new.setdefault("meta", {})
    new["schema_version"] = 1
    return new


register("scene", _scene_v0_to_v1, 0, 1)
register("scene", _scene_v1_to_v2, 1, 2)
register("flowscript", _flow_v1_to_v2, 1, 2)


def _cfg_v1_to_v2(data: dict) -> dict:
    new = dict(data)
    if "engine_version" in new:
        new.pop("engine_version")
    new["schema_version"] = 2
    return new

register("engine_cfg", _cfg_v0_to_v1, 0, 1)
register("engine_cfg", _cfg_v1_to_v2, 1, 2)
