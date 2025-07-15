from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, List

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


def register(widget: Any) -> None:
    _widgets.append(widget)


def set_theme(path: str) -> None:
    global current
    current = Theme.load(path)
    for w in list(_widgets):
        if hasattr(w, "apply_theme"):
            w.apply_theme()
