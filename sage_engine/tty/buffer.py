from __future__ import annotations

from dataclasses import dataclass, field

from sage_engine.color import Color, parse_color


@dataclass
class TTYCell:
    char: str = " "
    fg: Color = field(default_factory=lambda: Color(255, 255, 255))
    bg: Color = field(default_factory=lambda: Color(0, 0, 0))
    bold: bool = False


class TTYBuffer:
    """Simple 2D character buffer."""

    def __init__(self, width: int = 80, height: int = 25) -> None:
        self.width = width
        self.height = height
        self.cells: list[list[TTYCell]] = [
            [TTYCell() for _ in range(width)] for _ in range(height)
        ]

    def clear(self) -> None:
        for row in self.cells:
            for cell in row:
                cell.char = " "
                cell.fg = Color(255, 255, 255)
                cell.bg = Color(0, 0, 0)
                cell.bold = False

    def set_cell(
        self,
        x: int,
        y: int,
        char: str,
        fg: Color | str = Color(255, 255, 255),
        bg: Color | str = Color(0, 0, 0),
        bold: bool = False,
    ) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            cell = self.cells[y][x]
            cell.char = char
            cell.fg = parse_color(fg)
            cell.bg = parse_color(bg)
            cell.bold = bold
