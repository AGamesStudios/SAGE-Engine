"""Internal state container used by InputCore."""

class InputState:
    def __init__(self) -> None:
        # keys that were pressed this frame
        self.key_down: set[str] = set()
        # keys currently held down
        self.pressed_keys: set[str] = set()
        # keys released this frame
        self.key_up: set[str] = set()
        self.mouse_x = 0
        self.mouse_y = 0
        self.prev_x = 0
        self.prev_y = 0

