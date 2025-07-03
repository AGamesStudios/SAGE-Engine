"""Gizmo rendering helpers for the OpenGL renderer."""

import math

from OpenGL.GL import (
    glBindTexture, glColor4f, glBegin, glEnd, glVertex2f, glLineWidth,
    glPushMatrix, glPopMatrix, glTranslatef, glRotatef,
    GL_LINES, GL_TRIANGLES, GL_QUADS, GL_LINE_LOOP, GL_TEXTURE_2D,
)  # type: ignore[import-not-found]

from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import units


def draw_gizmo(
    renderer,
    obj: GameObject,
    camera: Camera | None,
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

    glLineWidth(base_w)
    glColor4f(0.7, 0.7, 1.0, 1.0 if mode == "rotate" else 0.5)
    steps = 24
    angle_step = 360.0 / steps
    glBegin(GL_LINE_LOOP)
    for i in range(steps):
        ang = math.radians(i * angle_step)
        glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
    glEnd()
    glLineWidth(ring_w)
    glPopMatrix()
