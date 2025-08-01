from __future__ import annotations

import json
import os
from pathlib import Path
from dataclasses import dataclass
from typing import Dict, Any

DEFAULT_THEME_NAME = "default"
_base_path = Path(__file__).resolve().parent.parent.parent
_default_theme_path = _base_path / "resources" / "default_theme.json"
if _default_theme_path.exists():
    with open(_default_theme_path, "r", encoding="utf8") as f:
        _raw_theme = json.load(f)
else:
    _raw_theme = {
        "font": "fonts/default.ttf",
        "font_size": 14,
        "color": [255, 255, 255, 255],
        "background": [0, 0, 0, 0],
    }

DEFAULT_THEME = {
    "font": os.path.join("resources", _raw_theme.get("font", "fonts/default.ttf")),
    "font_size": _raw_theme.get("font_size", 14),
    "fg_color": _raw_theme.get("color", [255, 255, 255, 255]),
    "bg_color": _raw_theme.get("background", [0, 0, 0, 0]),
}


@dataclass
class WidgetStyle:
    bg_color: tuple[int, int, int, int] = (40, 40, 40, 255)
    fg_color: tuple[int, int, int, int] = (255, 255, 255, 255)
    padding: int = 4
    border_color: tuple[int, int, int, int] = (0, 0, 0, 255)
    border_width: int = 0
    radius: int = 0
    font: str = "resources/fonts/default.ttf"
    font_size: int = 14
    hover_style: "WidgetStyle | None" = None
    focus_style: "WidgetStyle | None" = None


_themes: Dict[str, Dict[str, Any]] = {}


def load_theme(name: str, src: str | Dict[str, Any]) -> None:
    if isinstance(src, str) and os.path.exists(src):
        with open(src, "r", encoding="utf8") as f:
            data = json.load(f)
    elif isinstance(src, dict):
        data = src
    else:
        return

    vars: Dict[str, Any] = {k: v for k, v in data.items() if k.startswith("$")}
    styles: Dict[str, Any] = {k: v for k, v in data.items() if not k.startswith("$")}

    def resolve(val: Any) -> Any:
        if isinstance(val, str) and val.startswith("$"):
            return vars.get(val, val)
        return val

    for key, defs in styles.items():
        if isinstance(defs, dict):
            for k, v in list(defs.items()):
                if isinstance(v, dict):
                    defs[k] = {sk: resolve(sv) for sk, sv in v.items()}
                else:
                    defs[k] = resolve(v)
        else:
            styles[key] = resolve(defs)

    _themes[name] = styles


def apply_theme(style: WidgetStyle, theme: str) -> None:
    defs = _themes.get(theme)
    if not defs:
        return
    if "bg_color" in defs:
        style.bg_color = tuple(defs["bg_color"])
    if "fg_color" in defs:
        style.fg_color = tuple(defs["fg_color"])
    if "padding" in defs:
        style.padding = defs["padding"]
    if "border_color" in defs:
        style.border_color = tuple(defs["border_color"])
    if "border_width" in defs:
        style.border_width = defs["border_width"]
    if "radius" in defs:
        style.radius = defs["radius"]
    if "font" in defs:
        style.font = defs["font"]
    if "font_size" in defs:
        style.font_size = defs["font_size"]
    if "hover_style" in defs:
        style.hover_style = WidgetStyle()
        sub = defs["hover_style"]
        for k, v in sub.items():
            setattr(style.hover_style, k, v)
    if "focus_style" in defs:
        style.focus_style = WidgetStyle()
        sub = defs["focus_style"]
        for k, v in sub.items():
            setattr(style.focus_style, k, v)


load_theme(DEFAULT_THEME_NAME, DEFAULT_THEME)
