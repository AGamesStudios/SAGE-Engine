"""Data migration helpers for versionless files."""

from __future__ import annotations

from typing import Callable, Dict, List


# simple rule creators ------------------------------------------------------

def migrate_field(old: str, new: str) -> Callable[[dict], bool]:
    """Return a rule renaming *old* key to *new* in a mapping."""
    def apply(data: dict) -> bool:
        if old in data:
            data[new] = data.pop(old)
            return True
        return False
    return apply


def remove_field(key: str) -> Callable[[dict], bool]:
    """Return a rule removing *key* from a mapping if present."""
    def apply(data: dict) -> bool:
        return data.pop(key, None) is not None
    return apply


def set_default(key: str, value: object) -> Callable[[dict], bool]:
    """Return a rule setting *key* to *value* if missing."""
    def apply(data: dict) -> bool:
        if key not in data:
            data[key] = value
            return True
        return False
    return apply


def wrap(key: str, *, inside: str) -> Callable[[dict], bool]:
    """Return a rule moving *key* under a new mapping *inside*."""
    def apply(data: dict) -> bool:
        if key in data:
            value = data.pop(key)
            box = data.setdefault(inside, {})
            if isinstance(box, dict):
                box[key] = value
                return True
            data[key] = value  # rollback
        return False
    return apply


# declarative migration table ----------------------------------------------

MIGRATIONS: Dict[str, List[Callable[[dict], bool]]] = {
    "blueprint_object": [
        migrate_field("objectName", "name"),
        remove_field("deprecatedField"),
    ],
    "world": [
        migrate_field("entities", "objects"),
    ],
    "config": [
        migrate_field("screen_width", "width"),
        migrate_field("screen_height", "height"),
        set_default("fullscreen", False),
    ],
}

migrated_count = 0


def _guess_kind(data: dict) -> str:
    if "objects" in data or "entities" in data:
        return "world"
    if "render_backend" in data or "screen_width" in data:
        return "config"
    return "unknown"


def migrate(data: dict, kind: str | None = None) -> dict:
    """Apply migration rules to *data* in-place and return it."""
    global migrated_count
    if kind is None:
        kind = _guess_kind(data)
    rules = MIGRATIONS.get(kind, [])
    for rule in rules:
        if rule(data):
            migrated_count += 1
    return data


__all__ = [
    "migrate",
    "migrate_field",
    "remove_field",
    "set_default",
    "wrap",
    "MIGRATIONS",
    "migrated_count",
]
