"""FlowScript placeholder modules."""

from importlib import import_module

__all__ = ["python", "lua"]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.flow.{name}")
    raise AttributeError(name)
