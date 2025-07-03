
"""Simple library loader for the SAGE engine."""

import ctypes
import importlib
import importlib.util
import os

from ..utils.log import logger


class LibraryLoader:
    """Load Python modules or shared libraries from search paths."""

    def __init__(self, *, search_paths: list[str] | None = None) -> None:
        self.search_paths: list[str] = list(search_paths) if search_paths else []
        env = os.environ.get("SAGE_LIBRARY_PATH")
        if env:
            self.search_paths.extend([p for p in env.split(os.pathsep) if p])
        self.libraries: dict[str, object] = {}

    # internal helpers -----------------------------------------------------
    def _find_file(self, name: str) -> str | None:
        if os.path.isabs(name) and os.path.exists(name):
            return name
        for path in self.search_paths:
            candidate = os.path.join(path, name)
            if os.path.exists(candidate):
                return candidate
            for ext in (".py", ".so", ".dll", ".dylib"):
                if os.path.exists(candidate + ext):
                    return candidate + ext
        return None

    def _load_from_path(self, path: str):
        if path.endswith((".so", ".dll", ".dylib")):
            return ctypes.CDLL(path)
        spec = importlib.util.spec_from_file_location(os.path.basename(path), path)
        if spec and spec.loader:
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return module
        raise ImportError(f"Cannot load library from {path}")

    def _init_library(self, lib: object, instance: object | None) -> None:
        if instance is None:
            return
        init = getattr(lib, "init_engine", None) or getattr(lib, "init", None)
        if callable(init):
            try:
                init(instance)
            except Exception:
                logger.exception(
                    "Library init failed for %s", getattr(lib, "__name__", lib)
                )
                raise

    # public API -----------------------------------------------------------
    def load(self, name: str, instance: object | None = None):
        """Load a library by name and optionally initialize it with *instance*."""
        if name in self.libraries:
            lib = self.libraries[name]
        else:
            path = self._find_file(name)
            try:
                lib = (
                    self._load_from_path(path)
                    if path
                    else importlib.import_module(name)
                )
            except Exception:
                logger.exception("Failed to load library %s", name)
                raise
            self.libraries[name] = lib
        self._init_library(lib, instance)
        return lib

    def load_all(self, instance: object | None = None) -> None:
        """Load every library found in :attr:`search_paths`."""
        for path in self.search_paths:
            if not os.path.isdir(path):
                continue
            for name in os.listdir(path):
                if name.startswith("_"):
                    continue
                file_path = os.path.join(path, name)
                if os.path.isdir(file_path):
                    continue
                mod_name = os.path.splitext(name)[0]
                if mod_name in self.libraries:
                    continue
                try:
                    lib = self._load_from_path(file_path)
                    self.libraries[mod_name] = lib
                    self._init_library(lib, instance)
                except Exception:
                    logger.exception("Failed to load library %s", file_path)
                    raise

    def get(self, name: str) -> object | None:
        """Return a previously loaded library or ``None``."""
        return self.libraries.get(name)


DEFAULT_LIBRARY_LOADER = LibraryLoader()

def load_engine_libraries(engine: object, paths: list[str] | None = None) -> None:
    """Load engine libraries using the default loader."""
    if paths:
        for p in paths:
            if p not in DEFAULT_LIBRARY_LOADER.search_paths:
                DEFAULT_LIBRARY_LOADER.search_paths.append(p)
    DEFAULT_LIBRARY_LOADER.load_all(engine)
