"""Transform and coordinate conversion utilities.

This package provides lightweight 2D transform math and helpers for
converting coordinates between spaces such as world and screen.  The
implementation intentionally avoids external dependencies so the
modules can be used in headless test environments.
"""

from .types import (
    Coord,
    Rect,
    Space,
    Transform2D,
    BaseTransform,
    NodeTransform,
    RectangleTransform,
)
from .convert import (
    Camera2D,
    local_to_world,
    world_to_screen,
    screen_to_world,
    screen_rect_to_world,
    pixel_snap,
    snap_rect,
)
from .core import (
    prepare_world_all,
    get_local_aabb,
    get_world_aabb,
    get_screen_bounds,
    intersects_screen,
    collect_visible,
    serialize_transform,
    apply_transform,
    TransformCuller,
)

__all__ = [
    "Coord",
    "Rect",
    "Space",
    "Transform2D",
    "BaseTransform",
    "NodeTransform",
    "RectangleTransform",
    "Camera2D",
    "local_to_world",
    "world_to_screen",
    "screen_to_world",
    "screen_rect_to_world",
    "pixel_snap",
    "snap_rect",
    "prepare_world_all",
    "get_local_aabb",
    "get_world_aabb",
    "get_screen_bounds",
    "intersects_screen",
    "collect_visible",
    "serialize_transform",
    "apply_transform",
    "TransformCuller",
]
