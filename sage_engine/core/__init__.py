"""Minimal core loop for SAGE Engine."""

from __future__ import annotations

import logging
from .extensible import PhaseRegistry

registry = PhaseRegistry()
register = registry.register
expose = registry.expose
get = registry.get

_running = False


def stop() -> None:
    """Stop the main loop after current iteration."""
    global _running
    _running = False


def boot_engine(config: dict | None = None) -> None:
    """Boot engine and enter update loop until ``stop`` is called or
    ``KeyboardInterrupt`` is received."""
    logger = logging.getLogger("core")
    registry.run("boot", config or {})
    logger.info("Boot complete")
    logger.info("Entering update loop...")
    global _running
    _running = True
    try:
        while _running:
            registry.run("update")
            registry.run("draw")
            registry.run("flush")
    except KeyboardInterrupt:
        pass
    finally:
        registry.run("shutdown")
