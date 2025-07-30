"""Simple graphics API using a software framebuffer."""
from __future__ import annotations

from .backends import get_backend

_runtime = get_backend()

init = _runtime.init
begin_frame = _runtime.begin_frame
draw_rect = _runtime.draw_rect
draw_circle = _runtime.draw_circle
draw_line = _runtime.draw_line
draw_polygon = _runtime.draw_polygon
draw_rounded_rect = _runtime.draw_rounded_rect
draw_text = _runtime.draw_text
end_frame = _runtime.end_frame
flush_frame = _runtime.flush_frame
shutdown = _runtime.shutdown

add_effect = _runtime.add_effect
clear_effects = _runtime.clear_effects

state = _runtime.state
push_state = _runtime.push_state
pop_state = _runtime.pop_state

from ..graphic.scene import Scene, Layer, Rect, Group
from ..graphic.color import Color, to_rgba
from ..graphic import fx

