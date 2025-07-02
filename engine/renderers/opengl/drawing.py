"""Helper functions for drawing primitives with OpenGLRenderer."""
from __future__ import annotations

import math

from OpenGL.GL import (
    glBindTexture, glColor4f, glBegin, glEnd, glVertex2f, glLineWidth,
    glPushMatrix, glPopMatrix, glTranslatef,
    glTexCoord2f, glUseProgram,
    GL_LINES, GL_LINE_LOOP, GL_QUADS, GL_TRIANGLES, GL_TRIANGLE_FAN,
    GL_TEXTURE_2D,
)

from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import units
from engine.mesh_utils import (
    Mesh,
    create_square_mesh,
    create_triangle_mesh,
    create_circle_mesh,
)


# ---------------------------------------------------------------------------
# Color parsing
# ---------------------------------------------------------------------------

def parse_color(value) -> tuple[int, int, int, int]:
    """Return a 4-tuple RGBA color from various input formats."""
    if value is None:
        return 255, 255, 255, 255
    if isinstance(value, str):
        text = value.strip()
        try:
            if text.startswith("#"):
                text = text[1:]
                if len(text) in (3, 4):
                    parts = [int(c * 2, 16) for c in text]
                elif len(text) in (6, 8):
                    parts = [int(text[i:i+2], 16) for i in range(0, len(text), 2)]
                else:
                    raise ValueError
            else:
                text = text.replace(" ", ",")
                parts = [int(p) for p in text.split(",") if p]
            while len(parts) < 4:
                parts.append(255)
            return tuple(int(p) for p in parts[:4])
        except Exception:
            return 255, 255, 255, 255
    if isinstance(value, (list, tuple)):
        if len(value) == 3:
            return int(value[0]), int(value[1]), int(value[2]), 255
        if len(value) >= 4:
            return int(value[0]), int(value[1]), int(value[2]), int(value[3])
    return 255, 255, 255, 255


# ---------------------------------------------------------------------------
# Drawing helpers
# ---------------------------------------------------------------------------

def draw_icon(renderer, x: float, y: float, tex: int, zoom: float, size: float = 32.0) -> None:
    """Render a billboard icon at ``(x, y)`` in world units."""
    unit_scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    glBindTexture(GL_TEXTURE_2D, tex)
    glColor4f(1.0, 1.0, 1.0, 1.0)
    glPushMatrix()
    glTranslatef(x * unit_scale, y * unit_scale * sign, 0)
    inv_zoom = 1.0 / zoom if zoom else 1.0
    half = size / 2.0 * inv_zoom
    glBegin(GL_QUADS)
    glTexCoord2f(0.0, 0.0); glVertex2f(-half, -half)  # noqa: E702
    glTexCoord2f(1.0, 0.0); glVertex2f( half, -half)  # noqa: E702
    glTexCoord2f(1.0, 1.0); glVertex2f( half,  half)  # noqa: E702
    glTexCoord2f(0.0, 1.0); glVertex2f(-half,  half)  # noqa: E702
    glEnd()
    glPopMatrix()


def draw_outline(renderer, obj: GameObject, camera: Camera | None,
                 color: tuple[float, float, float, float] = (1.0, 0.5, 0.0, 1.0),
                 width: float = 3.0) -> None:
    """Draw a bright outline around ``obj`` in world coordinates."""
    if obj is None:
        return
    unit_scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    scale_mul = obj.render_scale(camera, apply_effects=renderer.apply_effects)
    obj_x, obj_y = obj.render_position(camera, apply_effects=renderer.apply_effects)
    ang = math.radians(getattr(obj, "angle", 0.0))
    cos_a = math.cos(ang)
    sin_a = math.sin(ang)
    sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
    sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
    w = obj.width * obj.scale_x * scale_mul
    h = obj.height * obj.scale_y * scale_mul
    px = w * obj.pivot_x
    py = h * obj.pivot_y
    corners = [
        (-px, -py),
        (w - px, -py),
        (w - px, h - py),
        (-px, h - py),
    ]
    glBindTexture(GL_TEXTURE_2D, 0)
    glColor4f(*color)
    glLineWidth(width)
    glBegin(GL_LINE_LOOP)
    for cx, cy in corners:
        vx = (cx - px) * sx + px
        vy = (cy - py) * sy + py
        rx = vx * cos_a - vy * sin_a
        ry = vx * sin_a + vy * cos_a
        world_x = (rx + obj_x) * unit_scale
        world_y = (ry + obj_y) * unit_scale * sign
        glVertex2f(world_x, world_y)
    glEnd()
    glLineWidth(1.0)


def draw_mesh(renderer, obj: GameObject, camera: Camera | None, mesh: Mesh) -> None:
    """Render a custom Mesh with the object's transform."""
    unit_scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    scale_mul = obj.render_scale(camera, apply_effects=renderer.apply_effects)
    glUseProgram(0)
    glBindTexture(GL_TEXTURE_2D, 0)

    rgba = parse_color(obj.color)
    scale = 1 / 255.0 if max(rgba) > 1.0 else 1.0
    alpha = getattr(obj, 'alpha', 1.0)
    if alpha > 1.0:
        alpha = alpha / 255.0
    glColor4f(
        rgba[0] * scale,
        rgba[1] * scale,
        rgba[2] * scale,
        min(1.0, rgba[3] * scale * alpha),
    )

    ang = math.radians(getattr(obj, "angle", 0.0))
    cos_a = math.cos(ang)
    sin_a = math.sin(ang)
    flip_x = getattr(obj, "flip_x", False)
    flip_y = getattr(obj, "flip_y", False)
    w = obj.width * obj.scale_x * scale_mul
    h = obj.height * obj.scale_y * scale_mul

    mode = GL_TRIANGLES if mesh.indices else GL_TRIANGLE_FAN
    obj_x, obj_y = obj.render_position(camera, apply_effects=renderer.apply_effects)
    px = w * obj.pivot_x
    py = h * obj.pivot_y
    sx = -1.0 if flip_x else 1.0
    sy = -1.0 if flip_y else 1.0

    glBegin(mode)
    verts = mesh.vertices
    indices = mesh.indices if mesh.indices else range(len(verts))
    for idx in indices:
        vx, vy = verts[idx]
        vx = (vx * w - px) * sx + px
        vy = (vy * h - py) * sy + py
        rx = vx * cos_a - vy * sin_a
        ry = vx * sin_a + vy * cos_a
        world_x = (rx + obj_x) * unit_scale
        world_y = (ry + obj_y) * unit_scale * sign
        glVertex2f(world_x, world_y)
    glEnd()


def draw_shape(renderer, obj: GameObject, camera: Camera | None, shape: str) -> None:
    if shape == "triangle":
        mesh = create_triangle_mesh()
    elif shape == "square":
        mesh = create_square_mesh()
    else:
        mesh = create_circle_mesh()
    draw_mesh(renderer, obj, camera, mesh)


def draw_frustum(renderer, cam: Camera, color=(1.0, 1.0, 0.0, 1.0), width: float = 1.0) -> None:
    left, bottom, w, h = cam.view_rect()
    sign = 1.0 if units.Y_UP else -1.0
    glBindTexture(GL_TEXTURE_2D, 0)
    glColor4f(*color)
    glLineWidth(width)
    glBegin(GL_LINE_LOOP)
    glVertex2f(left, bottom * sign)
    glVertex2f(left + w, bottom * sign)
    glVertex2f(left + w, (bottom + h) * sign)
    glVertex2f(left, (bottom + h) * sign)
    glEnd()
    glLineWidth(1.0)


def draw_origin(renderer, camera: Camera | None) -> None:
    if not camera:
        zoom = 1.0
        cam_x = cam_y = 0.0
    else:
        zoom = camera.zoom
        cam_x = camera.x
        cam_y = camera.y
    scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    half_w = renderer.width / 2 / (scale * zoom)
    half_h = renderer.height / 2 / (scale * zoom)
    start_x = cam_x - half_w
    end_x = cam_x + half_w
    start_y = cam_y - half_h
    end_y = cam_y + half_h
    glBindTexture(GL_TEXTURE_2D, 0)
    glBegin(GL_LINES)
    glColor4f(1.0, 0.0, 0.0, 1.0)
    glVertex2f(start_x * scale, 0.0)
    glVertex2f(end_x * scale, 0.0)
    glColor4f(0.0, 1.0, 0.0, 1.0)
    glVertex2f(0.0, start_y * scale * sign)
    glVertex2f(0.0, end_y * scale * sign)
    glEnd()


def draw_grid(renderer, camera: Camera | None) -> None:
    if not camera:
        zoom = 1.0
        cam_x = cam_y = 0.0
    else:
        zoom = camera.zoom
        cam_x = camera.x
        cam_y = camera.y
    spacing = renderer.grid_size
    if spacing <= 0:
        return
    pix_per_unit = units.UNITS_PER_METER * zoom * spacing
    target = 64.0
    while pix_per_unit < target / 2:
        spacing *= 2
        pix_per_unit *= 2
    while pix_per_unit > target * 2:
        spacing /= 2
        pix_per_unit /= 2
    scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    half_w = renderer.width / 2 / (scale * zoom)
    half_h = renderer.height / 2 / (scale * zoom)
    start_x = math.floor((cam_x - half_w) / spacing) * spacing
    end_x = math.ceil((cam_x + half_w) / spacing) * spacing
    start_y = math.floor((cam_y - half_h) / spacing) * spacing
    end_y = math.ceil((cam_y + half_h) / spacing) * spacing
    glBindTexture(GL_TEXTURE_2D, 0)
    glColor4f(*renderer.grid_color)
    glBegin(GL_LINES)
    x = start_x
    while x <= end_x:
        glVertex2f(x * scale, start_y * scale * sign)
        glVertex2f(x * scale, end_y * scale * sign)
        x += spacing
    y = start_y
    while y <= end_y:
        glVertex2f(start_x * scale, y * scale * sign)
        glVertex2f(end_x * scale, y * scale * sign)
        y += spacing
    glEnd()


def draw_cursor(renderer, x: float, y: float, camera: Camera | None) -> None:
    if camera is None:
        zoom = 1.0
    else:
        zoom = camera.zoom
    size = 5.0 / zoom
    scale = units.UNITS_PER_METER
    sign = 1.0 if units.Y_UP else -1.0
    glBindTexture(GL_TEXTURE_2D, 0)
    glPushMatrix()
    glTranslatef(x * scale, y * scale * sign, 0)
    glColor4f(1.0, 1.0, 1.0, 1.0)
    glBegin(GL_LINES)
    glVertex2f(-size, 0.0)
    glVertex2f(size, 0.0)
    glVertex2f(0.0, -size)
    glVertex2f(0.0, size)
    glEnd()
    glPopMatrix()
