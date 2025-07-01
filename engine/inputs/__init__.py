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


class InputManager:
    """Map high level actions to backend keys or buttons."""

    def __init__(self, backend: InputBackend):
        self.backend = backend
        self._key_map: dict[str, int] = {}
        self._btn_map: dict[str, int] = {}

    def bind_action(self, action: str, *, key: int | None = None,
                    button: int | None = None) -> None:
        """Bind ``action`` to ``key`` or ``button``."""
        if key is not None:
            self._key_map[action] = key
        if button is not None:
            self._btn_map[action] = button

    def unbind_action(self, action: str) -> None:
        self._key_map.pop(action, None)
        self._btn_map.pop(action, None)

    def poll(self) -> None:
        self.backend.poll()

    def is_action_down(self, action: str) -> bool:
        key = self._key_map.get(action)
        if key is not None and self.backend.is_key_down(key):
            return True
        button = self._btn_map.get(action)
        if button is not None and self.backend.is_button_down(button):
            return True
        return False

    def shutdown(self) -> None:
        self.backend.shutdown()


__all__ = [
    "InputBackend",
    "InputManager",
    "register_input",
    "get_input",
    "INPUT_REGISTRY",
]
