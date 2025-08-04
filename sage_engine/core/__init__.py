"""Minimal core loop for SAGE Engine."""

from __future__ import annotations

import logging
from .extensible import PhaseRegistry

registry = PhaseRegistry()
register = registry.register
expose = registry.expose
get = registry.get

_running = False
_logger = logging.getLogger("core")


def stop() -> None:
    """Stop the main loop after current iteration."""
    global _running
    _running = False


def safe_shutdown() -> None:
    """Public API for requesting engine shutdown."""
    stop()


def boot_engine(config: dict | None = None) -> None:
    """Boot engine and enter update loop until ``stop`` is called or
    ``KeyboardInterrupt`` is received."""
    registry.run("boot", config or {})
    _logger.info("Engine boot complete.")
    global _running
    _running = True
    try:
        while _running:
            registry.run("update")
            registry.run("draw")
            registry.run("flush")
            _logger.info("Frame complete.")
    except KeyboardInterrupt:
        pass
    finally:
        registry.run("shutdown")
