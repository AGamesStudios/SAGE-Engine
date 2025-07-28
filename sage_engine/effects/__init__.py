"""Public interface for SAGE Effects."""
from .api import register, apply, list_effects, set_backend, get_backend
from . import builtin  # register built-ins
from .pipeline import apply_pipeline
from .compat import convert

__all__ = [
    "register",
    "apply",
    "list_effects",
    "set_backend",
    "get_backend",
    "apply_pipeline",
    "convert",
]
