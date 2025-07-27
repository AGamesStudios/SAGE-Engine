"""Scheduling utilities combining time and timers."""

from importlib import import_module

__all__ = ["time", "timers"]

def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.scheduler.{name}")
    raise AttributeError(name)
