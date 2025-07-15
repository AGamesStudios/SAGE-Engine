# ruff: noqa
"""Compatibility alias for the renamed :mod:`sage_engine` package."""
from importlib import import_module
import sys

_sage = import_module("sage_engine")
__path__ = _sage.__path__
__spec__ = _sage.__spec__

def __getattr__(name):
    return getattr(_sage, name)

def __dir__():
    return dir(_sage)

sys.modules.setdefault(__name__, _sage)
