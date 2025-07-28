"""Compatibility and data migration helpers."""

from __future__ import annotations

from typing import Callable, Dict, List, Tuple

_registry: Dict[str, List[Tuple[str, str, Callable[[dict], dict]]]] = {}


def register(schema_type: str, from_version: str, to_version: str, func: Callable[[dict], dict]) -> None:
    """Register a migration *func* for *schema_type* from *from_version* to *to_version*."""
    _registry.setdefault(schema_type, []).append((from_version, to_version, func))


def migrate(schema_type: str, version: str, target: str, data: dict) -> tuple[str, dict]:
    """Migrate *data* from *version* to *target* applying registered migrations."""
    while version != target:
        for frm, to, func in _registry.get(schema_type, []):
            if frm == version:
                data = func(data)
                version = to
                break
        else:
            break
    return version, data

__all__ = ["register", "migrate"]
