from collections import defaultdict
from typing import Callable, Dict, List, Any


class PhaseRegistry:
    """Stores phase callbacks and exposed systems."""

    def __init__(self) -> None:
        self._phases: Dict[str, List[Callable[..., Any]]] = defaultdict(list)
        self._systems: Dict[str, Any] = {}

    def register(self, phase: str, func: Callable[..., Any]) -> None:
        """Register ``func`` to be called during ``phase``."""
        self._phases[phase].append(func)

    def run(self, phase: str, *args: Any, **kwargs: Any) -> None:
        """Execute all callbacks for ``phase``."""
        for func in self._phases.get(phase, []):
            func(*args, **kwargs)

    def expose(self, name: str, system: Any) -> None:
        """Expose a system under ``name``."""
        self._systems[name] = system

    def get(self, name: str) -> Any | None:
        """Retrieve previously exposed system."""
        return self._systems.get(name)
