from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Any

from .events import Event
from .style import WidgetStyle


@dataclass
class Widget:
    x: int = 0
    y: int = 0
    width: int = 10
    height: int = 10
    visible: bool = True
    focusable: bool = False
    style: WidgetStyle = field(default_factory=WidgetStyle)
    children: List["Widget"] = field(default_factory=list)
    parent: "Widget" | None = field(default=None, repr=False)

    on_click: Event = field(default_factory=Event, init=False)
    on_hover: Event = field(default_factory=Event, init=False)
    on_focus: Event = field(default_factory=Event, init=False)
    on_change: Event = field(default_factory=Event, init=False)
    on_keypress: Event = field(default_factory=Event, init=False)
    on_drag_start: Event = field(default_factory=Event, init=False)
    on_drag_move: Event = field(default_factory=Event, init=False)
    on_drag_end: Event = field(default_factory=Event, init=False)
    on_drop: Event = field(default_factory=Event, init=False)
    _bind_target: Any = field(default=None, init=False, repr=False)
    _bind_attr: str | None = field(default=None, init=False, repr=False)
    _bind_widget_field: str | None = field(default=None, init=False, repr=False)

    def __post_init__(self) -> None:
        from . import style
        style.apply_theme(self.style, style.DEFAULT_THEME_NAME)

    def validate(self) -> bool:
        from ..logger import logger
        valid = True
        if self.width is None or self.height is None:
            logger.error("[gui] Invalid size on %s", type(self).__name__)
            valid = False
        elif self.width < 0 or self.height < 0:
            logger.error("[gui] Negative size on %s", type(self).__name__)
            valid = False
        if self.x is None or self.y is None:
            logger.error("[gui] Invalid position on %s", type(self).__name__)
            valid = False
        return valid

    def contains_point(self, x: int, y: int) -> bool:
        return (
            self.x <= x < self.x + self.width and
            self.y <= y < self.y + self.height
        )

    def add_child(self, w: "Widget") -> None:
        w.parent = self
        from . import style
        style.apply_theme(w.style, style.DEFAULT_THEME_NAME)
        self.children.append(w)

    def bind(self, field: str, obj: Any) -> None:
        from ..logger import logger
        self._bind_attr = field
        self._bind_target = obj
        if not hasattr(obj, field):
            logger.error("[gui] bind: missing field %s on %s", field, type(obj).__name__)
            return
        widget_field = "value" if hasattr(self, "value") else "checked" if hasattr(self, "checked") else field
        if hasattr(self, widget_field):
            setattr(self, widget_field, getattr(obj, field))
        self.on_change.connect(lambda *a, **k: self._push_bind())
        self._bind_widget_field = widget_field

    def _push_bind(self) -> None:
        if self._bind_target and self._bind_attr:
            if hasattr(self._bind_target, self._bind_attr):
                val = getattr(self, getattr(self, "_bind_widget_field", self._bind_attr))
                setattr(self._bind_target, self._bind_attr, val)

    def pull_bind(self) -> None:
        if self._bind_target and self._bind_attr:
            if hasattr(self._bind_target, self._bind_attr):
                val = getattr(self._bind_target, self._bind_attr)
                setattr(self, getattr(self, "_bind_widget_field", self._bind_attr), val)

    def draw(self) -> None:
        if not self.visible:
            return
        if not self.validate():
            return
        from .. import gfx
        gfx.draw_rect(self.x, self.y, self.width, self.height, self.style.bg_color)
        for child in self.children:
            child.draw()
