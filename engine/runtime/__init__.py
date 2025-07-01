"""Runtime accessors grouped under ``engine.runtime``."""
from importlib import import_module

_engine = import_module('engine')
__all__ = getattr(_engine, '__all__', [])

def __getattr__(name: str):
    return getattr(_engine, name)
