from __future__ import annotations

from pathlib import Path

from sage_engine.color import load_theme


def load_ui_theme(name: str):
    path = Path("resources/themes") / f"{name}.json"
    return load_theme(path)
