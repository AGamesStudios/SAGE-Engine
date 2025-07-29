from __future__ import annotations

class InputCore:
    """Basic keyboard and mouse input handler."""

    def __init__(self) -> None:
        self._pressed: set[str] = set()
        self._held: set[str] = set()
        self._released: set[str] = set()
        self._mouse_x = 0
        self._mouse_y = 0
        self._prev_x = 0
        self._prev_y = 0
        self._actions: dict[str, str] = {}

    # internal event hooks
    def _handle_key(self, key: str, down: bool) -> None:
        if down:
            if key not in self._held:
                self._pressed.add(key)
                self._held.add(key)
        else:
            if key in self._held:
                self._held.remove(key)
                self._released.add(key)

    def _handle_mouse_move(self, x: int, y: int) -> None:
        self._mouse_x = x
        self._mouse_y = y

    # public API
    def poll(self) -> None:
        """Advance to next frame clearing transient states."""
        self._pressed.clear()
        self._released.clear()
        self._prev_x = self._mouse_x
        self._prev_y = self._mouse_y

    def is_pressed(self, key: str) -> bool:
        return key in self._held

    def was_pressed(self, key: str) -> bool:
        return key in self._pressed

    def was_released(self, key: str) -> bool:
        return key in self._released

    def get_mouse_position(self) -> tuple[int, int]:
        return self._mouse_x, self._mouse_y

    def get_mouse_delta(self) -> tuple[int, int]:
        return self._mouse_x - self._prev_x, self._mouse_y - self._prev_y

    def map_action(self, action: str, key: str | None = None, gamepad_button: str | None = None) -> None:
        if key:
            self._actions[action] = key

    def is_action(self, action: str) -> bool:
        key = self._actions.get(action)
        if not key:
            return False
        return self.is_pressed(key) or self.was_pressed(key)
