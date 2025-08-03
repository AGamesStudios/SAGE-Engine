"""Transform and coordinate conversion utilities.

This package provides lightweight 2D transform math and helpers for
converting coordinates between spaces such as world and screen.  The
implementation intentionally avoids external dependencies so the
modules can be used in headless test environments.
"""

from .types import Coord, Rect, Space, Transform2D, NodeTransform
from .convert import (
    Camera2D,
    local_to_world,
    world_to_screen,
    screen_to_world,
)
from .core import prepare_world_all

__all__ = [
    "Coord",
    "Rect",
    "Space",
    "Transform2D",
    "NodeTransform",
    "Camera2D",
    "local_to_world",
    "world_to_screen",
    "screen_to_world",
    "prepare_world_all",
]
