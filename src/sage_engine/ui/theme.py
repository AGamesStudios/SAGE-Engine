from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, List

import yaml


@dataclass
class Theme:
    colors: dict[str, str]
    font: dict[str, Any]
    radius: int

    @classmethod
    def load(cls, path: str | Path) -> "Theme":
        data = yaml.safe_load(Path(path).read_text())
        return cls(
            colors=data.get("colors", {}),
            font=data.get("font", {}),
            radius=data.get("radius", 0),
        )


_current_path = Path(__file__).with_name("default.vel")
current: Theme = Theme.load(_current_path)

_widgets: List[Any] = []


def register(widget: Any) -> None:
    _widgets.append(widget)


def set_theme(path: str) -> None:
    global current
    current = Theme.load(path)
    for w in list(_widgets):
        if hasattr(w, "apply_theme"):
            w.apply_theme()
