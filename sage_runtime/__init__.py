"""SAGE Runtime package exposing the core engine.

This wrapper allows using ``import sage_runtime`` as a drop-in replacement for
``import engine``. All attributes are proxied to the ``engine`` module so the
runtime can be embedded separately from the editor.
"""
from importlib import import_module

_engine = import_module("engine")
__all__ = getattr(_engine, "__all__", [])

def __getattr__(name: str):
    return getattr(_engine, name)
