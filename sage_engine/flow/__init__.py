"""SAGE FlowScript package."""

from importlib import import_module

__all__ = [
    "runtime",
    "parser",
    "compiler",
    "types",
    "bindings",
    "dialects",
]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.flow.{name}")
    raise AttributeError(name)
