"""Base class for SAGE GUI widgets."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, List, Tuple

from ..core import style
from ..core.event import Event


@dataclass
class BaseWidget:
    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0
    children: List["BaseWidget"] = field(default_factory=list)
    parent: "BaseWidget | None" = None
    visible: bool = True

    def update(self, ctx: Any, dt: float) -> None:
        for child in self.children:
            child.update(ctx, dt)

    def draw(self, ctx: Any) -> None:
        for child in self.children:
            child.draw(ctx)

    def handle_event(self, ctx: Any, event: Event) -> bool:
        for child in reversed(self.children):
            if child.handle_event(ctx, event):
                return True
        return False

    def add_child(self, widget: "BaseWidget") -> None:
        widget.parent = self
        self.children.append(widget)
