"""Simplified resource management."""

from __future__ import annotations

from pathlib import Path
import json

from sage_object import SAGEObject, object_from_dict


class ResourceManager:
    def __init__(self) -> None:
        self.paths: list[Path] = []
        self.loaded: set[Path] = set()

    def scan(self, root: str = "resources") -> None:
        root_path = Path(root)
        if root_path.is_dir():
            for entry in root_path.rglob("*"):
                if entry.is_file():
                    self.paths.append(entry)

    def load_all_required(self) -> None:
        for p in self.paths:
            self._load(p)

    def _load(self, path: Path) -> None:
        self.loaded.add(path)

    def load_objects(self, path: str) -> list[SAGEObject]:
        """Load SAGEObjects from a .sage_object file."""
        p = Path(path)
        with p.open("r", encoding="utf-8") as fh:
            data = json.load(fh)
        if data == []:
            return []
        if isinstance(data, dict):
            records = [data]
        elif isinstance(data, list):
            records = data
        else:
            raise ValueError("Invalid object file")
        objects: list[SAGEObject] = []
        for rec in records:
            if not isinstance(rec, dict):
                raise ValueError("Invalid object record")
            obj = object_from_dict(rec)
            objects.append(obj)
        return objects

    def load_all_objects(self, folder: str) -> list[SAGEObject]:
        """Recursively load all .sage_object files from *folder*."""
        root = Path(folder)
        objects: list[SAGEObject] = []
        if not root.exists():
            return objects
        for file in root.rglob("*.sage_object"):
            objects.extend(self.load_objects(str(file)))
        return objects


_manager: ResourceManager | None = None


def boot() -> None:
    """Initialise the resource subsystem."""
    global _manager
    _manager = ResourceManager()


def reset() -> None:
    global _manager
    _manager = None


def destroy() -> None:
    reset()


def get_manager() -> ResourceManager:
    assert _manager is not None, "ResourceManager not initialised"
    return _manager


# Backwards compatibility
init_resource = boot

__all__ = [
    "ResourceManager",
    "boot",
    "reset",
    "destroy",
    "get_manager",
    "init_resource",
]
