"""Public interface for SAGE Effects."""
from .api import (
    register,
    apply,
    list_effects,
    set_backend,
    get_backend,
    apply_pipeline,
    set_scissor,
    clear_scissor,
    set_mask,
    clear_mask,
    save_preset,
    load_preset,
    stats,
    Frame,
)
from . import builtin  # register built-ins
from .compat import convert

__all__ = [
    "register",
    "apply",
    "list_effects",
    "set_backend",
    "get_backend",
    "apply_pipeline",
    "convert",
    "set_scissor",
    "clear_scissor",
    "set_mask",
    "clear_mask",
    "save_preset",
    "load_preset",
    "stats",
    "Frame",
]
