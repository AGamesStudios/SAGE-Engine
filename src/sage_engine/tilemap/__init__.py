from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
import xml.etree.ElementTree as ET

from .. import physics


def _parse_csv(data: str) -> list[int]:
    return [int(x) for x in data.strip().split(',') if x]


@dataclass
class TileLayer:
    width: int
    height: int
    tiles: list[int]
    parallax_x: float = 1.0
    parallax_y: float = 1.0
    tile_width: int = 32
    tile_height: int = 32
    bodies: list[physics.Body] = field(default_factory=list)

    def draw_offset(self, cam_x: float, cam_y: float) -> tuple[float, float]:
        """Return drawing offset based on camera position and parallax."""
        return cam_x * self.parallax_x, cam_y * self.parallax_y

    def grid(self) -> list[tuple[int, int]]:
        """Return tile grid coordinates for debug rendering."""
        return [(x, y) for y in range(self.height) for x in range(self.width)]

    def grid_lines(self) -> list[tuple[float, float, float, float]]:
        """Return line segments covering the tile grid."""
        lines = []
        w = self.width * self.tile_width
        h = self.height * self.tile_height
        for x in range(self.width + 1):
            px = x * self.tile_width
            lines.append((px, 0, px, h))
        for y in range(self.height + 1):
            py = y * self.tile_height
            lines.append((0, py, w, py))
        return lines

    @classmethod
    def from_tmx(
        cls,
        path: str | Path,
        layer: int = 0,
        *,
        world: physics.World | None = None,
    ) -> "TileLayer":
        """Load one layer from a TMX map.

        If *world* is provided, tiles with ``collidable=true`` create static
        bodies in that physics world.
        """

        tree = ET.parse(path)
        root = tree.getroot()

        # gather collidable and one-way tile ids
        collidable: set[int] = set()
        one_way: set[int] = set()
        for ts in root.findall("tileset"):
            firstgid = int(ts.attrib.get("firstgid", "1"))
            for tile in ts.findall("tile"):
                props = tile.find("properties")
                if props is None:
                    continue
                for prop in props.findall("property"):
                    name = prop.attrib.get("name")
                    val = prop.attrib.get("value", "false")
                    if name == "collidable" and val in {"true", "1"}:
                        collidable.add(firstgid + int(tile.attrib.get("id", "0")))
                    if name == "one_way" and val in {"true", "1"}:
                        one_way.add(firstgid + int(tile.attrib.get("id", "0")))

        layers = root.findall("layer")
        if not layers:
            raise ValueError("no layers in map")
        lyr = layers[layer]

        width = int(lyr.attrib["width"])
        height = int(lyr.attrib["height"])
        parallax_x = float(
            lyr.attrib.get(
                "parallaxx", lyr.attrib.get("parallax_x", lyr.attrib.get("parallax", 1))
            )
        )
        parallax_y = float(
            lyr.attrib.get(
                "parallaxy", lyr.attrib.get("parallax_y", lyr.attrib.get("parallax", 1))
            )
        )
        props = lyr.find("properties")
        if props is not None:
            for prop in props.findall("property"):
                if prop.attrib.get("name") == "parallax":
                    val = float(prop.attrib.get("value", "1"))
                    parallax_x = parallax_y = val

        data = lyr.find("data")
        if data is None or data.get("encoding") != "csv":
            raise ValueError("only csv-encoded layers supported")
        tiles = _parse_csv(data.text or "")

        tilewidth = int(root.attrib.get("tilewidth", "32"))
        tileheight = int(root.attrib.get("tileheight", "32"))

        bodies: list[physics.Body] = []
        if world is not None:
            for y in range(height):
                for x in range(width):
                    gid = tiles[y * width + x]
                    if gid in collidable or gid in one_way:
                        behaviour = "one_way" if gid in one_way else "static"
                        bodies.append(
                            world.create_box(
                                x=x * tilewidth,
                                y=y * tileheight,
                                behaviour=behaviour,
                            )
                        )

        return cls(
            width,
            height,
            tiles,
            parallax_x,
            parallax_y,
            tilewidth,
            tileheight,
            bodies,
        )


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
