"""Simple DAG executor for engine phases."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable, Dict, Iterable, List

@dataclass
class Phase:
    name: str
    func: Callable
    deps: List[str] = field(default_factory=list)
    parallelizable: bool = False


_phases: Dict[str, Phase] = {}


def add_phase(name: str, func: Callable, *, deps: Iterable[str] | None = None, parallelizable: bool = False) -> None:
    """Register a new phase in the DAG."""
    if name in _phases:
        raise ValueError(f"Phase '{name}' already registered")
    _phases[name] = Phase(name, func, list(deps or []), parallelizable)


def ordered_phases() -> List[Phase]:
    """Return phases sorted topologically by dependencies."""
    order: List[Phase] = []
    temp: Dict[str, bool] = {}

    def visit(ph: Phase) -> None:
        if ph.name in temp:
            if not temp[ph.name]:
                raise RuntimeError(f"Cycle detected at {ph.name}")
            return
        temp[ph.name] = False
        for dep in ph.deps:
            if dep not in _phases:
                continue
            visit(_phases[dep])
        temp[ph.name] = True
        order.append(ph)

    for ph in list(_phases.values()):
        if ph.name not in temp:
            visit(ph)
    return order


def run() -> None:
    """Execute all phases respecting dependencies."""
    for ph in ordered_phases():
        ph.func()


def reset() -> None:
    _phases.clear()
