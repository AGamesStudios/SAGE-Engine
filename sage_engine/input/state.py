"""Internal state container used by InputCore."""

class InputState:
    def __init__(self) -> None:
        self.pressed: set[str] = set()
        self.held: set[str] = set()
        self.released: set[str] = set()
        self.mouse_x = 0
        self.mouse_y = 0
        self.prev_x = 0
        self.prev_y = 0
