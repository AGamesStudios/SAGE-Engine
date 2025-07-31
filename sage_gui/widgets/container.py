"""Layout container widgets."""

from dataclasses import dataclass

from typing import List
from .base_widget import BaseWidget


@dataclass
class VBox(BaseWidget):
    spacing: int = 4

    def layout(self) -> None:
        y = self.y
        for child in self.children:
            child.x = self.x
            child.y = y
            y += child.height + self.spacing

    def update(self, ctx, dt: float) -> None:
        self.layout()
        super().update(ctx, dt)


@dataclass
class HBox(BaseWidget):
    spacing: int = 4

    def layout(self) -> None:
        x = self.x
        for child in self.children:
            child.x = x
            child.y = self.y
            x += child.width + self.spacing

    def update(self, ctx, dt: float) -> None:
        self.layout()
        super().update(ctx, dt)
