from __future__ import annotations

class CursorState:
    """Manage cursor visibility and position."""

    def __init__(self) -> None:
        self.x = 0
        self.y = 0
        self.visible = True
        self.style = "default"
        self.follow_rate = 1.0

    def set_position(self, x: int, y: int) -> None:
        if self.follow_rate >= 1.0:
            self.x = x
            self.y = y
        else:
            self.x += int((x - self.x) * self.follow_rate)
            self.y += int((y - self.y) * self.follow_rate)

    def get_position(self) -> tuple[int, int]:
        return self.x, self.y

    def set_visible(self, flag: bool) -> None:
        self.visible = flag

    def set_style(self, name: str) -> None:
        self.style = name
