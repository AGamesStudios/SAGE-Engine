"""Core module providing basic engine loop and dependency registry."""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Dict, List

from ..format.loader import load_sage_file
from importlib import import_module

from ..settings import settings
from ..logger import logger
import asyncio
from concurrent.futures import ThreadPoolExecutor

@dataclass
class _Phase:
    func: Callable
    parallelizable: bool = False


_registry: Dict[str, List[_Phase]] = defaultdict(list)
_booted = False


def _load_modules_from_config() -> None:
    path = Path(__file__).resolve().parents[2] / "engine.sagecfg"
    if not path.exists():
        return
    data = load_sage_file(path)
    settings.features.update(data.get("features", {}))
    settings_dict = data.get("settings", {})
    for k, v in settings_dict.items():
        if hasattr(settings, k):
            setattr(settings, k, v)
    for mod in data.get("modules", []):
        try:
            import_module(f"sage_engine.{mod}")
        except ModuleNotFoundError:
            continue


def register(phase: str, func: Callable, *, parallelizable: bool = False) -> None:
    """Register a callable for execution in a given phase."""
    _registry[phase].append(_Phase(func, parallelizable))


def core_boot(config: dict | None = None) -> None:
    """Boot the engine core and all modules."""
    global _booted
    if _booted:
        return
    _booted = True
    _load_modules_from_config()
    logger.phase_func = lambda: "boot"
    # load role definitions before booting modules
    from .. import roles
    roles.load_json_roles(docs_path=Path(__file__).resolve().parents[2] / "docs" / "roles.md")
    for phase in _registry.get("boot", []):
        phase.func(config or {})


async def core_boot_async(config: dict | None = None) -> None:
    """Asynchronously boot the engine core."""
    global _booted
    if _booted:
        return
    _booted = True
    _load_modules_from_config()
    loop = asyncio.get_event_loop()
    tasks = [
        loop.run_in_executor(None, ph.func, config or {})
        for ph in _registry.get("boot", [])
    ]
    if tasks:
        await asyncio.gather(*tasks)


def core_tick() -> None:
    """Execute a single frame by running all phases in order."""
    if not _booted:
        raise RuntimeError("Engine not booted")
    for phase_name in ("update", "draw", "flush"):
        logger.phase_func = lambda pn=phase_name: pn
        phases = _registry.get(phase_name, [])
        serial = [p for p in phases if not p.parallelizable or not settings.enable_multithread]
        parallel = [p for p in phases if p.parallelizable and settings.enable_multithread]

        for p in serial:
            p.func()

        if parallel:
            with ThreadPoolExecutor(max_workers=settings.cpu_threads) as ex:
                list(ex.map(lambda ph: ph.func(), parallel))


def core_reset() -> None:
    """Reset engine state while keeping modules alive."""
    for phase in _registry.get("reset", []):
        phase.func()


def core_shutdown() -> None:
    """Shutdown the engine core."""
    global _booted
    for phase in _registry.get("shutdown", []):
        phase.func()
    _booted = False
    _registry.clear()
