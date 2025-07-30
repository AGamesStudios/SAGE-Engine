"""FlowScript bytecode helpers."""

from importlib import import_module

__all__ = ["encoder", "vm"]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.flow.bytecode.{name}")
    raise AttributeError(name)
