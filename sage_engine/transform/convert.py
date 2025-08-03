"""Coordinate space conversion helpers."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Tuple

from .math2d import apply_to_point, identity, multiply
from .types import Coord, Space, Transform2D


@dataclass
class Camera2D:
    pos: Tuple[float, float] = (0.0, 0.0)
    rot: float = 0.0
    zoom: float = 1.0
    viewport_px: Tuple[int, int] = (800, 600)


def local_to_world(node, coord: Coord) -> Coord:
    # NodeTransform exposes ``world_matrix`` which handles parent recursion.
    m = node.world_matrix()
    x, y = apply_to_point(m, coord.x, coord.y)
    return Coord(x, y, Space.WORLD)


def world_to_screen(camera: Camera2D, coord_world: Coord) -> Coord:
    px, py = coord_world.x - camera.pos[0], coord_world.y - camera.pos[1]
    sx = px * camera.zoom + camera.viewport_px[0] / 2
    sy = camera.viewport_px[1] / 2 - py * camera.zoom
    return Coord(sx, sy, Space.SCREEN)


def screen_to_world(camera: Camera2D, coord_screen: Coord) -> Coord:
    px = (coord_screen.x - camera.viewport_px[0] / 2) / camera.zoom + camera.pos[0]
    py = (camera.viewport_px[1] / 2 - coord_screen.y) / camera.zoom + camera.pos[1]
    return Coord(px, py, Space.WORLD)
