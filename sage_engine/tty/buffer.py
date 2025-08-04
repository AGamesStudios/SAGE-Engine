from __future__ import annotations

from dataclasses import dataclass


@dataclass
class TTYCell:
    char: str = " "
    fg: str = "white"
    bg: str = "black"
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
                cell.fg = "white"
                cell.bg = "black"
                cell.bold = False

    def set_cell(
        self,
        x: int,
        y: int,
        char: str,
        fg: str = "white",
        bg: str = "black",
        bold: bool = False,
    ) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            cell = self.cells[y][x]
            cell.char = char
            cell.fg = fg
            cell.bg = bg
            cell.bold = bold
