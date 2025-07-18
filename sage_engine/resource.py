"""Simplified resource management."""

from __future__ import annotations

from pathlib import Path


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


_manager: ResourceManager | None = None


def init_resource() -> None:
    global _manager
    _manager = ResourceManager()


def get_manager() -> ResourceManager:
    assert _manager is not None, "ResourceManager not initialised"
    return _manager
