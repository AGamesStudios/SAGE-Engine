from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import xml.etree.ElementTree as ET


def _parse_csv(data: str) -> list[int]:
    return [int(x) for x in data.strip().split(',') if x]


@dataclass
class TileLayer:
    width: int
    height: int
    tiles: list[int]
    parallax: float = 1.0

    @classmethod
    def from_tmx(cls, path: str | Path, layer: int = 0, *, parallax: float = 1.0) -> "TileLayer":
        tree = ET.parse(path)
        root = tree.getroot()
        layers = root.findall('layer')
        if not layers:
            raise ValueError('no layers in map')
        lyr = layers[layer]
        width = int(lyr.attrib['width'])
        height = int(lyr.attrib['height'])
        data = lyr.find('data')
        if data is None or data.get('encoding') != 'csv':
            raise ValueError('only csv-encoded layers supported')
        tiles = _parse_csv(data.text or '')
        return cls(width, height, tiles, parallax=parallax)


def autowang(layer: TileLayer, offset: int = 1) -> None:
    w, h = layer.width, layer.height
    src = layer.tiles
    dst = src.copy()

    def filled(x: int, y: int) -> bool:
        return 0 <= x < w and 0 <= y < h and src[y * w + x] > 0

    for y in range(h):
        for x in range(w):
            if src[y * w + x] <= 0:
                continue
            bits = (
                (1 if filled(x, y - 1) else 0) |
                (2 if filled(x + 1, y) else 0) |
                (4 if filled(x, y + 1) else 0) |
                (8 if filled(x - 1, y) else 0)
            )
            dst[y * w + x] = offset + bits
    layer.tiles = dst


__all__ = ["TileLayer", "autowang"]
