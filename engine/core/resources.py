# Simple resource management for SAGE Engine
import os
from ..log import logger

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
        self._cache: dict[str, bytes] = {}
        os.makedirs(self.root, exist_ok=True)
        logger.info("Resource manager initialized at %s", self.root)

    def path(self, *parts: str) -> str:
        return os.path.join(self.root, *parts)

    def add_folder(self, *parts: str) -> str:
        """Create a subfolder and return its path."""
        p = self.path(*parts)
        os.makedirs(p, exist_ok=True)
        logger.info("Created resource folder %s", p)
        return p

    def move(self, src: str, dst: str) -> None:
        src_path = self.path(src)
        dst_path = self.path(dst)
        os.makedirs(os.path.dirname(dst_path), exist_ok=True)
        os.rename(src_path, dst_path)
        logger.info("Moved resource %s -> %s", src, dst)

    def list(self, rel: str = "") -> list[str]:
        base = self.path(rel)
        if not os.path.exists(base):
            return []
        return sorted(os.listdir(base))

    def import_file(self, src: str, dest: str | None = None) -> str:
        """Copy ``src`` into the resource tree and return its relative path."""
        import shutil

        if dest is None:
            dest = self.root
        else:
            dest = self.path(dest)
        os.makedirs(dest, exist_ok=True)

        base = os.path.basename(src)
        name, ext = os.path.splitext(base)
        target = os.path.join(dest, base)
        counter = 1
        while os.path.exists(target):
            target = os.path.join(dest, f"{name}_{counter}{ext}")
            counter += 1
        shutil.copy(src, target)
        logger.info("Imported resource %s -> %s", src, target)
        if target.startswith(self.root):
            return os.path.relpath(target, self.root)
        return target

    def import_folder(self, src: str, dest: str | None = None) -> str:
        """Recursively copy a folder into the resources tree."""
        import shutil

        if dest is None:
            dest = self.root
        else:
            dest = self.path(dest)
        os.makedirs(dest, exist_ok=True)

        base = os.path.basename(os.path.normpath(src))
        target = os.path.join(dest, base)
        counter = 1
        while os.path.exists(target):
            target = os.path.join(dest, f"{base}_{counter}")
            counter += 1
        try:
            shutil.copytree(src, target)
            logger.info("Imported folder %s -> %s", src, target)
        except Exception:
            logger.exception("Failed to import folder %s", src)
            raise
        if target.startswith(self.root):
            return os.path.relpath(target, self.root)
        return target

    def load_data(self, rel_path: str) -> bytes:
        """Return the contents of a resource and cache it."""
        path = self.path(rel_path)
        with open(path, 'rb') as fh:
            data = fh.read()
        self._cache[rel_path] = data
        logger.info("Loaded resource %s (%d bytes)", rel_path, len(data))
        return data

    def get_cached(self, rel_path: str) -> bytes | None:
        """Return cached data if available."""
        return self._cache.get(rel_path)

    def clear_cache(self) -> None:
        """Empty the internal resource cache."""
        self._cache.clear()
