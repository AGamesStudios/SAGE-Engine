"""Loader for .sageanimation files."""

from __future__ import annotations

import json
import os

from ..animation import Animation, Frame

__all__ = ["load_sageanimation", "save_sageanimation"]


def load_sageanimation(path: str) -> Animation:
    """Load an animation from ``path``."""
    with open(path, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    frames = [
        Frame(f["image"], f.get("duration", 0.1))
        for f in data.get("frames", [])
    ]
    loop = data.get("loop", True)
    return Animation(frames, loop)


def save_sageanimation(anim: Animation, path: str) -> None:
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    data = {
        "loop": anim.loop,
        "frames": [
            {"image": fr.image, "duration": fr.duration} for fr in anim.frames
        ],
    }
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=2)


