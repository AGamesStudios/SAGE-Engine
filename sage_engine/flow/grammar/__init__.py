"""FlowScript grammar utilities."""

from importlib import import_module

__all__ = ["parser", "tokenizer"]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.flow.grammar.{name}")
    raise AttributeError(name)
