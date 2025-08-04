from __future__ import annotations

import json
from pathlib import Path

from .parser import parse_color


class Theme(dict):
    @classmethod
    def from_file(cls, path: str | Path) -> "Theme":
        data = json.loads(Path(path).read_text())
        theme = cls()
        for name, value in data.items():
            theme[name] = parse_color(value)
        return theme


def load_theme(path: str | Path) -> Theme:
    return Theme.from_file(path)
