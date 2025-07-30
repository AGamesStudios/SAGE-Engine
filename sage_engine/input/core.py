from __future__ import annotations

from .state import InputState

class InputCore:
    """Basic keyboard and mouse input handler."""

    def __init__(self) -> None:
        self._state = InputState()
        self._actions: dict[str, str] = {}

    # internal event hooks
    def _handle_key(self, key: str, down: bool) -> None:
        if down:
            if key not in self._state.pressed_keys:
                self._state.key_down.add(key)
                self._state.pressed_keys.add(key)
        else:
            if key in self._state.pressed_keys:
                self._state.pressed_keys.remove(key)
                self._state.key_up.add(key)

    def _handle_mouse_move(self, x: int, y: int) -> None:
        self._state.mouse_x = x
        self._state.mouse_y = y

    # public API
    def poll(self) -> None:
        """Advance to next frame clearing transient states."""
        self._state.key_down.clear()
        self._state.key_up.clear()
        self._state.prev_x = self._state.mouse_x
        self._state.prev_y = self._state.mouse_y

    def is_pressed(self, key: str) -> bool:
        """Return True while the key is held down."""
        return key in self._state.pressed_keys

    def is_down(self, key: str) -> bool:
        """Return True only on the frame the key was pressed."""
        return key in self._state.key_down

    def is_up(self, key: str) -> bool:
        """Return True only on the frame the key was released."""
        return key in self._state.key_up

    # Backwards compatibility
    was_pressed = is_down
    was_released = is_up

    def get_mouse_position(self) -> tuple[int, int]:
        return self._state.mouse_x, self._state.mouse_y

    mouse_position = get_mouse_position

    def get_mouse_delta(self) -> tuple[int, int]:
        return self._state.mouse_x - self._state.prev_x, self._state.mouse_y - self._state.prev_y

    def map_action(self, action: str, key: str | None = None, gamepad_button: str | None = None) -> None:
        from .state import KEY_MAP
        from ..logger import logger

        if key:
            key = key.upper()
            if key not in KEY_MAP:
                logger.warn("[input] Unknown key: %s", key)
                return
            self._actions[action] = key

    def unmap_action(self, action: str) -> None:
        if action in self._actions:
            del self._actions[action]

    def is_action(self, action: str) -> bool:
        key = self._actions.get(action)
        if not key:
            return False
        return self.is_pressed(key) or self.is_down(key)
