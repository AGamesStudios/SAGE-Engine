"""Runtime accessors grouped under ``engine.runtime``."""
from importlib import import_module

_engine = import_module("engine")
__all__ = getattr(_engine, "__all__", [])
__version__ = getattr(_engine, "__version__", "0")

# expose ``main`` directly for static analysis tools
from engine.core.engine import main as _main  # noqa: E402
main = _main
if "main" not in __all__:
    __all__.append("main")

def __getattr__(name: str):
    return getattr(_engine, name)
