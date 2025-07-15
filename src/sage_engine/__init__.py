"""SAGE Engine core package."""
__all__ = [
    "core",
    "patches",
    "render",
    "audio",
    "ui",
    "gui",
    "physics",
    "build",
    "soundmint",
    "tilemap",
    "ink",
    "sprites",
]

from importlib import import_module

for _name in __all__:
    globals()[_name] = import_module(f"{__name__}.{_name}")
