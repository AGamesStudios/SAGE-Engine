from __future__ import annotations

from dataclasses import dataclass

from sage_engine.gui.base import Widget
from sage_engine.gui import widgets


def _auto_field(name: str, value):
    if isinstance(value, bool):
        cb = widgets.Checkbox(checked=value)
        return cb
    if isinstance(value, (int, float, str)):
        inp = widgets.TextInput(text=str(value))
        return inp
    return widgets.Label(text=str(value))


@dataclass
class InspectorPanel(Widget):
    target: object | None = None

    def bind_object(self, obj: object) -> None:
        self.target = obj
        self.children.clear()
        for k in dir(obj):
            if k.startswith("_"):
                continue
            v = getattr(obj, k)
            field_widget = _auto_field(k, v)
            self.add_child(field_widget)
            field_widget.on_change.connect(lambda val, key=k: setattr(obj, key, val))
