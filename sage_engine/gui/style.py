from __future__ import annotations

import json
import os
from dataclasses import dataclass
from typing import Dict


@dataclass
class WidgetStyle:
    bg_color: tuple[int, int, int, int] = (40, 40, 40, 255)
    fg_color: tuple[int, int, int, int] = (255, 255, 255, 255)
    padding: int = 4
    border_color: tuple[int, int, int, int] = (0, 0, 0, 255)
    border_width: int = 0
    radius: int = 0


_themes: Dict[str, Dict[str, tuple[int, int, int, int]]] = {}


def load_theme(name: str, path: str) -> None:
    with open(path, "r", encoding="utf8") as f:
        data = json.load(f)
    _themes[name] = {
        k: tuple(v) for k, v in data.items()
    }


def apply_theme(style: WidgetStyle, theme: str) -> None:
    colors = _themes.get(theme)
    if not colors:
        return
    if "bg_color" in colors:
        style.bg_color = colors["bg_color"]
    if "fg_color" in colors:
        style.fg_color = colors["fg_color"]
