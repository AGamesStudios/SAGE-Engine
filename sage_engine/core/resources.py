# Simple resource management for SAGE Engine
import os

_RESOURCE_ROOT = ""


def set_resource_root(path: str) -> None:
    """Set the base directory for resource lookups."""
    global _RESOURCE_ROOT
    _RESOURCE_ROOT = path


def get_resource_path(name: str) -> str:
    """Return an absolute path for a resource name."""
    if os.path.isabs(name) or not _RESOURCE_ROOT:
        return name
    return os.path.join(_RESOURCE_ROOT, name)


class ResourceManager:
    """Utility class to manage resource files."""

    def __init__(self, root: str):
        self.root = root
        os.makedirs(self.root, exist_ok=True)

    def path(self, *parts: str) -> str:
        return os.path.join(self.root, *parts)

    def add_folder(self, name: str) -> str:
        p = self.path(name)
        os.makedirs(p, exist_ok=True)
        return p

    def move(self, src: str, dst: str) -> None:
        os.rename(self.path(src), self.path(dst))

    def list(self, rel: str = "") -> list[str]:
        base = self.path(rel)
        if not os.path.exists(base):
            return []
        return sorted(os.listdir(base))
