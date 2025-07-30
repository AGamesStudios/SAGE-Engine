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
            if key not in self._state.held:
                self._state.pressed.add(key)
                self._state.held.add(key)
        else:
            if key in self._state.held:
                self._state.held.remove(key)
                self._state.released.add(key)

    def _handle_mouse_move(self, x: int, y: int) -> None:
        self._state.mouse_x = x
        self._state.mouse_y = y

    # public API
    def poll(self) -> None:
        """Advance to next frame clearing transient states."""
        self._state.pressed.clear()
        self._state.released.clear()
        self._state.prev_x = self._state.mouse_x
        self._state.prev_y = self._state.mouse_y

    def is_pressed(self, key: str) -> bool:
        """Return True while the key is held down."""
        return key in self._state.held

    def is_down(self, key: str) -> bool:
        """Return True only on the frame the key was pressed."""
        return key in self._state.pressed

    def is_up(self, key: str) -> bool:
        """Return True only on the frame the key was released."""
        return key in self._state.released

    # Backwards compatibility
    was_pressed = is_down
    was_released = is_up

    def get_mouse_position(self) -> tuple[int, int]:
        return self._state.mouse_x, self._state.mouse_y

    mouse_position = get_mouse_position

    def get_mouse_delta(self) -> tuple[int, int]:
        return self._state.mouse_x - self._state.prev_x, self._state.mouse_y - self._state.prev_y

    def map_action(self, action: str, key: str | None = None, gamepad_button: str | None = None) -> None:
        if key:
            self._actions[action] = key

    def unmap_action(self, action: str) -> None:
        if action in self._actions:
            del self._actions[action]

    def is_action(self, action: str) -> bool:
        key = self._actions.get(action)
        if not key:
            return False
        return self.is_pressed(key) or self.is_down(key)
