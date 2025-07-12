"""Gizmo rendering helpers for the OpenGL renderer."""

import math

from OpenGL.GL import (
    glBindTexture, glColor4f, glBegin, glEnd, glVertex2f, glLineWidth,
    glPushMatrix, glPopMatrix, glTranslatef, glRotatef,
    GL_LINES, GL_TRIANGLES, GL_QUADS, GL_LINE_LOOP, GL_LINE_STRIP,
    GL_TEXTURE_2D, GL_TRIANGLE_FAN,
)  # type: ignore[import-not-found]

from typing import Optional

from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import units
from engine.gizmos import Gizmo


def draw_gizmo(
    renderer,
    obj: GameObject,
    camera: Optional[Camera],
    hover: str | None = None,
    dragging: str | None = None,
    mode: str = "move",
    local: bool = False,
) -> None:
    """Draw a transform gizmo for ``obj``."""
    if obj is None or mode == "pan":
        return
    scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    zoom = camera.zoom if camera else 1.0
    inv = 1.0 / zoom if zoom else 1.0
    ratio = renderer.widget.devicePixelRatioF() if renderer.widget else 1.0
    size = 50 * inv
    head = 10 * inv
    sq = 6 * inv
    ring_r = size * 1.2
    ring_w = 4 * ratio
    glBindTexture(GL_TEXTURE_2D, 0)
    glPushMatrix()
    glTranslatef(obj.x * scale, obj.y * scale * sign, 0)
    if local or mode == "scale":
        glRotatef(getattr(obj, "angle", 0.0), 0, 0, 1)
    base_w = 6 * ratio
    glLineWidth(base_w)
    if mode == "move":
        hx = hover in ("x", "xy") or dragging in ("x", "xy")
        color_x = 1.0 if hx else 0.7
        glColor4f(color_x, 0.0, 0.0, 1.0)
        glLineWidth(base_w * (1.5 if hx else 1.0))
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(size - head, 0.0)
        glEnd()

        hxy = hover == "xy" or dragging == "xy"
        color_xy = (1.0, 1.0, 0.0, 1.0) if hxy else (0.8, 0.8, 0.0, 1.0)
        glColor4f(*color_xy)
        sq_size = sq * (1.5 if hxy else 1.0)
        off = size * 0.3
        glBegin(GL_QUADS)
        glVertex2f(off - sq_size, sign * off - sq_size)
        glVertex2f(off + sq_size, sign * off - sq_size)
        glVertex2f(off + sq_size, sign * off + sq_size)
        glVertex2f(off - sq_size, sign * off + sq_size)
        glEnd()
        glBegin(GL_TRIANGLES)
        glVertex2f(size, 0.0)
        glVertex2f(size - head, head / 2)
        glVertex2f(size - head, -head / 2)
        glEnd()

        hy = hover in ("y", "xy") or dragging in ("y", "xy")
        color_y = 1.0 if hy else 0.7
        glColor4f(0.0, color_y, 0.0, 1.0)
        glLineWidth(base_w * (1.5 if hy else 1.0))
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(0.0, sign * (size - head))
        glEnd()
        glBegin(GL_TRIANGLES)
        if units.Y_UP:
            glVertex2f(0.0, size)
            glVertex2f(-head / 2, size - head)
            glVertex2f(head / 2, size - head)
        else:
            glVertex2f(0.0, -size)
            glVertex2f(-head / 2, -size + head)
            glVertex2f(head / 2, -size + head)
        glEnd()

    elif mode == "scale":
        sx_hover = hover == "sx" or dragging == "sx"
        color_sx = 1.0 if sx_hover else 0.7
        glColor4f(color_sx, 0.0, 0.0, 1.0)
        glLineWidth(base_w * (1.5 if sx_hover else 1.0))
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(size, 0.0)
        glEnd()
        sq_size = sq * (1.5 if sx_hover else 1.0)
        glBegin(GL_QUADS)
        glVertex2f(size, -sq_size)
        glVertex2f(size + 2 * sq_size, -sq_size)
        glVertex2f(size + 2 * sq_size, sq_size)
        glVertex2f(size, sq_size)
        glEnd()

        sy_hover = hover == "sy" or dragging == "sy"
        color_sy = 1.0 if sy_hover else 0.7
        glColor4f(0.0, color_sy, 0.0, 1.0)
        glLineWidth(base_w * (1.5 if sy_hover else 1.0))
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(0.0, size * sign)
        glEnd()
        sq_size = sq * (1.5 if sy_hover else 1.0)
        glBegin(GL_QUADS)
        if units.Y_UP:
            glVertex2f(-sq_size, size)
            glVertex2f(sq_size, size)
            glVertex2f(sq_size, size + 2 * sq_size)
            glVertex2f(-sq_size, size + 2 * sq_size)
        else:
            glVertex2f(-sq_size, -size - 2 * sq_size)
            glVertex2f(sq_size, -size - 2 * sq_size)
            glVertex2f(sq_size, -size)
            glVertex2f(-sq_size, -size)
        glEnd()

    if mode == "rotate":
        glLineWidth(base_w)
        glColor4f(0.7, 0.7, 1.0, 1.0)
        steps = 24
        angle_step = 360.0 / steps
        glBegin(GL_LINE_LOOP)
        for i in range(steps):
            ang = math.radians(i * angle_step)
            glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
        glEnd()
        glLineWidth(ring_w)
    glLineWidth(1.0)
    glPopMatrix()


def draw_basic_gizmo(renderer, gizmo: Gizmo, camera: Optional[Camera]) -> None:
    """Draw a simple gizmo shape using OpenGL primitives."""
    zoom = camera.zoom if camera else 1.0
    size = gizmo.size / zoom
    scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    glBindTexture(GL_TEXTURE_2D, 0)
    glPushMatrix()
    if gizmo.shape.lower() != "polyline":
        glTranslatef(gizmo.x * scale, gizmo.y * scale * sign, 0)
    color = tuple(max(0.0, min(1.0, c)) for c in gizmo.color)
    glColor4f(*color)
    glLineWidth(max(1.0, gizmo.thickness))
    shape = gizmo.shape.lower()
    if shape == "polyline" and gizmo.vertices:
        glBegin(GL_LINE_STRIP)
        for vx, vy in gizmo.vertices:
            glVertex2f(vx * scale, vy * scale * sign)
        glEnd()
    elif shape == "square":
        if gizmo.filled:
            glBegin(GL_QUADS)
            glVertex2f(-size, -size)
            glVertex2f(size, -size)
            glVertex2f(size, size)
            glVertex2f(-size, size)
            glEnd()
        else:
            glBegin(GL_LINE_LOOP)
            glVertex2f(-size, -size)
            glVertex2f(size, -size)
            glVertex2f(size, size)
            glVertex2f(-size, size)
            glEnd()
    elif shape == "circle":
        steps = max(16, int(size))
        if gizmo.filled:
            glBegin(GL_TRIANGLE_FAN)
            glVertex2f(0.0, 0.0)
            for i in range(steps + 1):
                ang = math.radians(i * 360.0 / steps)
                glVertex2f(math.cos(ang) * size, math.sin(ang) * size * sign)
            glEnd()
        else:
            glBegin(GL_LINE_LOOP)
            for i in range(steps):
                ang = math.radians(i * 360.0 / steps)
                glVertex2f(math.cos(ang) * size, math.sin(ang) * size * sign)
            glEnd()
    else:  # cross
        glBegin(GL_LINES)
        glVertex2f(-size, 0.0)
        glVertex2f(size, 0.0)
        glVertex2f(0.0, -size)
        glVertex2f(0.0, size)
        glEnd()
    glLineWidth(1.0)
    glPopMatrix()

__all__ = ['draw_gizmo', 'draw_basic_gizmo']
