"""Simple graphics API using a software framebuffer."""
from __future__ import annotations

from .backends import get_backend

_runtime = get_backend()

init = _runtime.init
begin_frame = _runtime.begin_frame
draw_rect = _runtime.draw_rect
end_frame = _runtime.end_frame
shutdown = _runtime.shutdown

add_effect = _runtime.add_effect
clear_effects = _runtime.clear_effects

from ..graphic.scene import Scene, Layer, Rect
from ..graphic.color import Color, to_rgba
from ..graphic import fx
