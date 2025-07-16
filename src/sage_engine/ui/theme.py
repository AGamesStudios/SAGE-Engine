from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, List, Dict, Optional

from ..render import font as _font
from ..render.font import Font

try:  # pragma: no cover - optional dependency
    import yaml
except ModuleNotFoundError:  # pragma: no cover - fallback parser
    yaml = None  # type: ignore


def _parse_simple(path: Path) -> dict[str, Any]:
    data: dict[str, Any] = {"colors": {}, "font": {}, "radius": 0}
    current = None
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line:
            continue
        if line.endswith(":"):
            key = line[:-1]
            current = data.setdefault(key, {})
            continue
        key, value = line.split(":", 1)
        value = value.strip().strip("'").strip('"')
        if current is None:
            data[key] = value
        elif key == "radius":
            data[key] = int(value)
        else:
            current[key] = int(value) if value.isdigit() else value
    return data


@dataclass
class Theme:
    colors: dict[str, str]
    font: dict[str, Any]
    radius: int

    @classmethod
    def load(cls, path: str | Path) -> "Theme":
        if yaml is not None:
            data = yaml.safe_load(Path(path).read_text())
        else:
            data = _parse_simple(Path(path))
        return cls(
            colors=data.get("colors", {}),
            font=data.get("font", {}),
            radius=data.get("radius", 0),
        )


_current_path = Path(__file__).with_name("default.vel")
current: Theme = Theme.load(_current_path)

_widgets: List[Any] = []
_font_cache: Dict[str, "Font"] = {}


def get_font() -> Optional[Font]:
    """Return the font defined in the current theme if available."""
    path = current.font.get("file")
    if not path:
        return None
    font = _font_cache.get(path)
    if font is None:
        try:
            font = _font.load(path)
            _font_cache[path] = font
        except Exception:
            return None
    return font


def color_rgba(hex_color: str) -> tuple[float, float, float, float]:
    """Convert ``#RRGGBB`` or ``#RRGGBBAA`` to floats."""
    c = hex_color.lstrip("#")
    if len(c) == 6:
        r, g, b = int(c[0:2], 16), int(c[2:4], 16), int(c[4:6], 16)
        a = 255
    else:
        r, g, b, a = (
            int(c[0:2], 16),
            int(c[2:4], 16),
            int(c[4:6], 16),
            int(c[6:8], 16),
        )
    return r / 255.0, g / 255.0, b / 255.0, a / 255.0


def register(widget: Any) -> None:
    _widgets.append(widget)


def set_theme(path: str) -> None:
    global current
    current = Theme.load(path)
    _font_cache.clear()
    for w in list(_widgets):
        if hasattr(w, "apply_theme"):
            w.apply_theme()


__all__ = ["Theme", "current", "set_theme", "color_rgba", "register", "get_font"]
