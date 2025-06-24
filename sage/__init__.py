"""Unified access to the SAGE engine and editor.

Both submodules are loaded lazily so projects can depend on one
without pulling in the heavy dependencies of the other.
"""

from importlib import import_module
import types


class _Lazy(types.ModuleType):
    """Lazy import wrapper for optional subpackages."""

    def __init__(self, name: str):
        super().__init__(name)
        self._name = name
        self._mod = None

    def _load(self):
        if self._mod is None:
            self._mod = import_module(self._name)

    def __getattr__(self, item):
        self._load()
        return getattr(self._mod, item)

    def __dir__(self):
        self._load()
        return dir(self._mod)


engine = _Lazy('sage.engine')
editor = _Lazy('sage.editor')

__all__ = ['engine', 'editor']
