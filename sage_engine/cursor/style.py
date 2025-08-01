from __future__ import annotations

from dataclasses import dataclass


@dataclass
class CursorStyle:
    name: str = "default"
    sprite_path: str = "resources/system/default_cursor.sageimg"
    hotspot: tuple[int, int] = (0, 0)
    animation: str = "none"


def load_style(cfg: dict) -> CursorStyle:
    return CursorStyle(
        name=cfg.get("cursor_style", "default"),
        sprite_path=cfg.get("sprite_path", "resources/system/default_cursor.sageimg"),
        hotspot=tuple(cfg.get("hotspot", (0, 0))),
        animation=cfg.get("animation", "none"),
    )
