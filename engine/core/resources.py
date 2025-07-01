# Simple resource management for SAGE Engine
import os
import sys
from typing import Callable
from ..utils.log import logger
from ..cache import SAGE_CACHE

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
        # cache already loaded files
        self._cache = {}
        os.makedirs(self._win_path(self.root), exist_ok=True)
        logger.info("Resource manager initialized at %s", self.root)

    # ------------------------------------------------------------------
    @staticmethod
    def _win_path(path: str) -> str:
        """Return *path* with the Windows long path prefix if needed."""
        if os.name == "nt":
            abs_path = os.path.abspath(path)
            if not abs_path.startswith("\\\\?\\"):
                return "\\\\?\\" + abs_path
            return abs_path
        return path

    # ------------------------------------------------------------------
    def _unique_file(self, dest: str, name: str) -> str:
        """Return a non-conflicting path under *dest* for *name*."""
        base, ext = os.path.splitext(name)
        target = os.path.join(dest, name)
        counter = 1
        while os.path.exists(target):
            target = os.path.join(dest, f"{base}_{counter}{ext}")
            counter += 1
        return target

    def _unique_dir(self, dest: str, name: str) -> str:
        """Return a non-conflicting folder path under *dest* named *name*."""
        target = os.path.join(dest, name)
        counter = 1
        while os.path.exists(target):
            target = os.path.join(dest, f"{name}_{counter}")
            counter += 1
        return target

    def path(self, *parts: str) -> str:
        return os.path.join(self.root, *parts)

    def add_folder(self, *parts: str) -> str:
        """Create a subfolder and return its path."""
        p = self.path(*parts)
        os.makedirs(self._win_path(p), exist_ok=True)
        logger.info("Created resource folder %s", p)
        return p

    def move(self, src: str, dst: str) -> None:
        src_path = self.path(src)
        dst_path = self.path(dst)
        if not os.path.exists(self._win_path(src_path)):
            logger.warning("Source missing for move %s -> %s", src, dst)
            return
        try:
            os.makedirs(self._win_path(os.path.dirname(dst_path)), exist_ok=True)
        except Exception:
            logger.exception("Failed creating folder for %s", dst)
            raise
        os.rename(self._win_path(src_path), self._win_path(dst_path))
        logger.info("Moved resource %s -> %s", src, dst)

    def list(self, rel: str = "") -> list[str]:
        base = self.path(rel)
        wbase = self._win_path(base)
        if not os.path.exists(wbase):
            return []
        return sorted(os.listdir(wbase))

    def import_file(
        self,
        src: str,
        dest: str | None = None,
        progress_cb: Callable[[int, str | None], None] | None = None,
    ) -> str:
        """Copy ``src`` into the resources and report progress via ``progress_cb``."""

        if dest is None:
            dest = self.root
        else:
            dest = self.path(dest)
        os.makedirs(self._win_path(dest), exist_ok=True)

        if src.lower().endswith('.zip'):
            return self.import_zip(src, dest, progress_cb)

        target = self._unique_file(dest, os.path.basename(src))

        try:
            with open(self._win_path(src), "rb") as fsrc, open(self._win_path(target), "wb") as fdst:
                while True:
                    chunk = fsrc.read(1024 * 1024)
                    if not chunk:
                        break
                    fdst.write(chunk)
                    if progress_cb:
                        progress_cb(len(chunk), os.path.relpath(target, self.root))
            logger.info("Imported resource %s -> %s", src, target)
        except Exception:
            logger.exception("Failed to import %s", src)
            raise

        if target.startswith(self.root):
            return os.path.relpath(target, self.root)
        return target

    def import_folder(
        self,
        src: str,
        dest: str | None = None,
        progress_cb: Callable[[int, str | None], None] | None = None,
    ) -> str:
        """Recursively copy a folder into the resources tree reporting progress."""

        if dest is None:
            dest = self.root
        else:
            dest = self.path(dest)
        os.makedirs(self._win_path(dest), exist_ok=True)

        target = self._unique_dir(dest, os.path.basename(os.path.normpath(src)))

        try:
            for root_dir, dirs, files in os.walk(src):
                rel_root = os.path.relpath(root_dir, src)
                dest_root = os.path.join(target, rel_root) if rel_root != "." else target
                os.makedirs(self._win_path(dest_root), exist_ok=True)
                for d in dirs:
                    os.makedirs(self._win_path(os.path.join(dest_root, d)), exist_ok=True)
                for f in files:
                    sfile = os.path.join(root_dir, f)
                    dfile = self._unique_file(dest_root, f)
                    try:
                        with open(self._win_path(sfile), "rb") as fin, open(self._win_path(dfile), "wb") as fout:
                            while True:
                                chunk = fin.read(1024 * 1024)
                                if not chunk:
                                    break
                                fout.write(chunk)
                                if progress_cb:
                                    progress_cb(len(chunk), os.path.relpath(dfile, self.root))
                    except Exception:
                        logger.exception("Failed copying %s", sfile)
            logger.info("Imported folder %s -> %s", src, target)
        except Exception:
            logger.exception("Failed to import folder %s", src)
            raise

        if target.startswith(self.root):
            return os.path.relpath(target, self.root)
        return target

    def import_zip(
        self,
        src: str,
        dest: str | None = None,
        progress_cb: Callable[[int, str | None], None] | None = None,
    ) -> str:
        """Extract ``src`` zip archive into the resources tree."""

        if dest is None:
            dest = self.root
        else:
            dest = self.path(dest)
        os.makedirs(self._win_path(dest), exist_ok=True)

        from zipfile import ZipFile

        base_name = os.path.splitext(os.path.basename(src))[0]
        target = self._unique_dir(dest, base_name)

        try:
            with ZipFile(self._win_path(src)) as zf:
                for info in zf.infolist():
                    out_path = os.path.join(target, info.filename)
                    if info.is_dir():
                        os.makedirs(self._win_path(out_path), exist_ok=True)
                        continue
                    os.makedirs(self._win_path(os.path.dirname(out_path)), exist_ok=True)
                    try:
                        with zf.open(info) as fin, open(self._win_path(out_path), "wb") as fout:
                            while True:
                                chunk = fin.read(1024 * 1024)
                                if not chunk:
                                    break
                                fout.write(chunk)
                                if progress_cb:
                                    progress_cb(len(chunk), os.path.relpath(out_path, self.root))
                    except Exception:
                        logger.exception("Failed extracting %s", info.filename)
            logger.info("Imported zip %s -> %s", src, target)
        except Exception:
            logger.exception("Failed to import zip %s", src)
            raise

        if target.startswith(self.root):
            return os.path.relpath(target, self.root)
        return target

    def load_data(self, rel_path: str) -> bytes:
        """Return the contents of a resource and cache it."""
        path = self.path(rel_path)
        data = SAGE_CACHE.get(path)
        if data is None:
            with open(self._win_path(path), 'rb') as fh:
                data = fh.read()
            SAGE_CACHE.set(path, data)
            logger.info("Loaded resource %s (%d bytes)", rel_path, len(data))
        self._cache[rel_path] = data
        return data

    def get_cached(self, rel_path: str) -> bytes | None:
        """Return cached data if available."""
        return self._cache.get(rel_path)

    def clear_cache(self) -> None:
        """Empty the internal resource cache."""
        self._cache.clear()
