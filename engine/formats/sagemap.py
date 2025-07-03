from __future__ import annotations

import json
import os
from typing import TypedDict, List


class SageMap(TypedDict):
    tileset: str
    tile_width: int
    tile_height: int
    width: int
    height: int
    data: List[int]


REQUIRED_FIELDS = [
    "tileset",
    "tile_width",
    "tile_height",
    "width",
    "height",
    "data",
]


def load_sagemap(path: str) -> SageMap:
    """Load a tile map from ``path``."""
    with open(path, "r", encoding="utf-8") as fh:
        data = json.load(fh)
    for field in REQUIRED_FIELDS:
        if field not in data:
            raise ValueError(f"Missing '{field}' field")
    if len(data["data"]) != data["width"] * data["height"]:
        raise ValueError("Data length does not match width * height")
    return data  # type: ignore[return-value]


def save_sagemap(map_data: SageMap, path: str) -> None:
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(map_data, fh, indent=2)

