"""Core module providing basic engine loop and dependency registry."""

from collections import defaultdict
from typing import Callable, Dict, List

_registry: Dict[str, List[Callable]] = defaultdict(list)
_booted = False


def register(phase: str, func: Callable) -> None:
    """Register a callable for execution in a given phase."""
    _registry[phase].append(func)


def core_boot(config: dict | None = None) -> None:
    """Boot the engine core and all modules."""
    global _booted
    if _booted:
        return
    _booted = True
    for func in _registry.get("boot", []):
        func(config or {})


def core_tick() -> None:
    """Execute a single frame by running all phases in order."""
    if not _booted:
        raise RuntimeError("Engine not booted")
    for phase in ("update", "draw", "flush"):
        for func in _registry.get(phase, []):
            func()


def core_reset() -> None:
    """Reset engine state while keeping modules alive."""
    for func in _registry.get("reset", []):
        func()


def core_shutdown() -> None:
    """Shutdown the engine core."""
    global _booted
    for func in _registry.get("shutdown", []):
        func()
    _booted = False
    _registry.clear()
