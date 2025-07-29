from __future__ import annotations

from .core import CursorState
from .style import CursorStyle
from ..graphic import api as graphic


class CursorRenderer:
    """Draws the cursor as overlay in ``graphic.flush``."""

    def __init__(self) -> None:
        self.state = CursorState()
        self.style = CursorStyle()

    def draw(self) -> None:
        if not self.state.visible:
            return
        sprite = {"path": self.style.sprite_path}
        x, y = self.state.get_position()
        graphic.draw_sprite(sprite, x - self.style.hotspot[0], y - self.style.hotspot[1])
