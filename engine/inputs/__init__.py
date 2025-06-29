"""Input backend registry and interface for the engine."""

from __future__ import annotations

from typing import Dict, Type
from abc import abstractmethod
from importlib import metadata
import logging


class InputBackend:
    """Abstract base class for input backends."""

    @abstractmethod
    def poll(self) -> None:
        """Process pending input events."""
        raise NotImplementedError

    @abstractmethod
    def is_key_down(self, key: int) -> bool:
        """Return True if *key* is currently pressed."""
        raise NotImplementedError

    @abstractmethod
    def is_button_down(self, button: int) -> bool:
        """Return True if *button* is currently pressed."""
        raise NotImplementedError

    @abstractmethod
    def shutdown(self) -> None:
        """Clean up any resources used by the backend."""
        raise NotImplementedError


logger = logging.getLogger(__name__)

INPUT_REGISTRY: Dict[str, Type] = {}
_PLUGINS_LOADED = False


def register_input(name: str, cls: Type) -> None:
    """Register an input backend class under ``name``."""
    INPUT_REGISTRY[name] = cls


def _load_entry_points() -> None:
    """Load input backends exposed via entry points."""
    global _PLUGINS_LOADED
    if _PLUGINS_LOADED:
        return
    try:
        eps = metadata.entry_points()
        entries = eps.select(group="sage_engine.inputs") if hasattr(eps, "select") else eps.get("sage_engine.inputs", [])
        for ep in entries:
            try:
                cls = ep.load()
                register_input(ep.name, cls)
            except Exception:
                logger.exception("Failed to load input backend %s", ep.name)
    except Exception:
        logger.exception("Error loading input entry points")
    _PLUGINS_LOADED = True


def get_input(name: str) -> Type | None:
    """Return the input backend class associated with ``name``."""
    _load_entry_points()
    return INPUT_REGISTRY.get(name)


__all__ = ["InputBackend", "register_input", "get_input", "INPUT_REGISTRY"]
