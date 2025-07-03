"""Input backend registry and interface for the engine."""


from typing import Dict, Type
from abc import ABC, abstractmethod
from importlib import metadata
import importlib
import logging



class InputBackend(ABC):
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

    def get_axis_value(self, axis_id: int) -> float | None:
        """Return the current analog value for ``axis_id`` in the range ``-1.0`` to ``1.0``.

        Return ``None`` if the backend does not provide the requested axis.
        """
        return None

    @abstractmethod
    def shutdown(self) -> None:
        """Clean up any resources used by the backend."""
        raise NotImplementedError


logger = logging.getLogger(__name__)

INPUT_REGISTRY: Dict[str, Type] = {}
_PLUGINS_LOADED = False


def _ensure_default() -> None:
    """Register built-in backends, falling back to NullInput."""
    if "null" not in INPUT_REGISTRY:
        from .null_input import NullInput
        register_input("null", NullInput)
    if "sdl" not in INPUT_REGISTRY:
        try:
            from ..core.input_sdl import SDLInput
        except Exception as exc:  # pragma: no cover - optional dependency
            logger.warning("SDL backend unavailable: %s", exc)
            from .null_input import NullInput
            register_input("sdl", NullInput)
        else:
            register_input("sdl", SDLInput)
    if "qt" not in INPUT_REGISTRY:
        try:
            from .qt_input import QtInput
        except Exception as exc:  # pragma: no cover - optional dependency
            logger.warning("Qt backend unavailable: %s", exc)
        else:
            register_input("qt", QtInput)
    _load_entry_points()


def register_input(name: str, cls: Type) -> None:
    """Register an input backend class under ``name``."""
    INPUT_REGISTRY[name] = cls


def _load_entry_points() -> None:
    """Load input backends exposed via entry points."""
    global _PLUGINS_LOADED
    if _PLUGINS_LOADED:
        return
    failed: list[str] = []
    try:
        eps = metadata.entry_points()
        entries = (
            eps.select(group="sage_engine.inputs")
            if hasattr(eps, "select")
            else eps.get("sage_engine.inputs", [])
        )
        for ep in entries:
            try:
                cls = ep.load()
                register_input(ep.name, cls)
            except Exception as exc:  # pragma: no cover - plugin may raise anything
                logger.exception("Failed to load input backend %s: %s", ep.name, exc)
                failed.append(ep.name)
    except Exception as exc:  # pragma: no cover - metadata issues
        logger.exception("Error loading input entry points: %s", exc)
        raise
    if failed:
        msg = ", ".join(failed)
        logger.warning("Failed input plugins: %s", msg)
        raise RuntimeError(f"Failed input plugins: {msg}")
    _PLUGINS_LOADED = True
    logger.info("Input backends available: %d", len(INPUT_REGISTRY))


def get_input(name: str) -> Type | None:
    """Return the input backend class associated with ``name``."""
    _ensure_default()
    return INPUT_REGISTRY.get(name)


class InputManager:
    """Map high level actions to backend keys or buttons and dispatch callbacks."""

    def __init__(self, backend: InputBackend):
        self.backend = backend
        self._key_map: dict[str, int] = {}
        self._btn_map: dict[str, int] = {}
        self._axes: dict[str, tuple[int | None, int | None, float, int | None]] = {}
        self._press: dict[str, list[callable]] = {}
        self._release: dict[str, list[callable]] = {}
        self._state: set[str] = set()

    def bind_action(self, action: str, *, key: int | None = None,
                    button: int | None = None) -> None:
        """Bind ``action`` to ``key`` or ``button``."""
        if key is not None:
            self._key_map[action] = key
        if button is not None:
            self._btn_map[action] = button

    def bind_axis(
        self,
        axis: str,
        *,
        positive: int | None = None,
        negative: int | None = None,
        axis_id: int | None = None,
        scale: float = 1.0,
    ) -> None:
        """Map ``axis`` to keys/buttons or an analog ``axis_id``."""
        self._axes[axis] = (positive, negative, scale, axis_id)

    def unbind_axis(self, axis: str) -> None:
        self._axes.pop(axis, None)

    def unbind_action(self, action: str) -> None:
        self._key_map.pop(action, None)
        self._btn_map.pop(action, None)

    def poll(self) -> None:
        self.backend.poll()
        actions = set(self._key_map) | set(self._btn_map)
        for action in actions:
            down = self.is_action_down(action)
            if down and action not in self._state:
                for cb in self._press.get(action, []):
                    cb()
            elif not down and action in self._state:
                for cb in self._release.get(action, []):
                    cb()
            if down:
                self._state.add(action)
            else:
                self._state.discard(action)

    def is_action_down(self, action: str) -> bool:
        key = self._key_map.get(action)
        if key is not None and self.backend.is_key_down(key):
            return True
        button = self._btn_map.get(action)
        if button is not None and self.backend.is_button_down(button):
            return True
        return False

    def get_axis(self, axis: str) -> float:
        pos, neg, scale, aid = self._axes.get(axis, (None, None, 1.0, None))
        value = 0.0
        if aid is not None:
            res = self.backend.get_axis_value(aid)
            if res is not None:
                value = res
        if pos is not None and self.backend.is_key_down(pos):
            value += scale
        if neg is not None and self.backend.is_key_down(neg):
            value -= scale
        return value

    def shutdown(self) -> None:
        self.backend.shutdown()

    def on_press(self, action: str, callback) -> None:
        self._press.setdefault(action, []).append(callback)

    def on_release(self, action: str, callback) -> None:
        self._release.setdefault(action, []).append(callback)


__all__ = [
    "InputBackend",
    "InputManager",
    "NullInput",
    "QtInput",
    "GamepadInput",
    "register_input",
    "get_input",
    "INPUT_REGISTRY",
]

NullInput = importlib.import_module(".null_input", __name__).NullInput
try:
    QtInput = importlib.import_module(".qt_input", __name__).QtInput
except Exception:  # pragma: no cover - PyQt6 missing
    QtInput = None  # type: ignore
try:
    GamepadInput = importlib.import_module(".gamepad", __name__).GamepadInput
except Exception:  # pragma: no cover - SDL2 missing
    GamepadInput = None  # type: ignore
