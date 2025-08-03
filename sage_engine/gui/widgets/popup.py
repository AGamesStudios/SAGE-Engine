from __future__ import annotations

from dataclasses import dataclass

from sage_engine.gui.base import Widget
from sage_engine.gui.events import Event


@dataclass
class Popup(Widget):
    """Popup that closes when clicked outside."""
    on_close: Event = Event()

    def close(self) -> None:
        self.visible = False
        self.on_close.emit()
