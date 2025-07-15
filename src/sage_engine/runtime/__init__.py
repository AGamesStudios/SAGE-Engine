"""Runtime accessors grouped under ``engine.runtime``."""
from importlib import import_module

_engine = import_module("engine")
__all__ = getattr(_engine, "__all__", [])
__version__ = getattr(_engine, "__version__", "0")

# expose ``main`` directly for static analysis tools
from sage_engine.core.engine import main as _main  # noqa: E402
main = _main
if "main" not in __all__:
    __all__.append("main")

__all__.append("get_attr")


def get_attr(name: str):
    """Return an attribute from the core ``engine`` module."""
    try:
        return getattr(_engine, name)
    except AttributeError:
        return getattr(_engine, "get_engine_attr")(name)
