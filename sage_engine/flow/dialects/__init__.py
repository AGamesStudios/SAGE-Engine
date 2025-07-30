"""Available FlowScript dialect compilers."""

from importlib import import_module

__all__ = ["flow_lang", "python_like", "lua_like"]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.flow.dialects.{name}")
    raise AttributeError(name)
