from __future__ import annotations

from dataclasses import dataclass

from ..base import Widget


@dataclass
class DockPanel(Widget):
    dock: str = "left"  # left, right, top, bottom
