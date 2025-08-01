from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from . import style
from .event import Event


@dataclass
class Widget:
    x: int = 0
    y: int = 0
    width: int = 10
    height: int = 10
    style: style.WidgetStyle = field(default_factory=style.WidgetStyle)
    children: List['Widget'] = field(default_factory=list)

    on_click: Event = field(default_factory=Event, init=False)
    on_hover: Event = field(default_factory=Event, init=False)

    def add_child(self, w: 'Widget') -> None:
        self.children.append(w)

    def draw(self) -> None:
        from .. import gfx
        gfx.draw_rect(self.x, self.y, self.width, self.height, self.style.bg_color)
        for child in self.children:
            child.draw()


@dataclass
class Label(Widget):
    text: str = ""

    def draw(self) -> None:
        super().draw()
        from .. import gfx
        gfx.draw_text(
            self.x + self.style.padding,
            self.y + self.style.padding,
            self.text,
            None,
            self.style.fg_color,
        )


@dataclass
class Button(Widget):
    text: str = ""

    def draw(self) -> None:
        super().draw()
        from .. import gfx
        gfx.draw_text(
            self.x + self.style.padding,
            self.y + self.style.padding,
            self.text,
            None,
            self.style.fg_color,
        )

    def click(self) -> None:
        self.on_click.emit()


class Container(Widget):
    pass


class Panel(Container):
    pass

