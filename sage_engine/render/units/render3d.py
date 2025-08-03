"""Software 3D rendering unit."""
from __future__ import annotations

import time
from typing import Tuple

from ...graphics.math3d import Vector3, Matrix4
from ...graphics.mesh3d import Mesh3D
from ...core import get as core_get
from ..zbuffer import ZBuffer
from .. import stats as render_stats
from ...logger import logger


class Render3DUnit:
    """Minimal CPU rasterizer for triangle meshes."""

    def __init__(self, runtime, zbuffer: ZBuffer | None = None) -> None:
        self.runtime = runtime
        self.zbuffer = zbuffer or ZBuffer(runtime.width, runtime.height)

    def resize(self, width: int, height: int) -> None:
        if width != self.zbuffer.width or height != self.zbuffer.height:
            self.zbuffer = ZBuffer(width, height)

    def draw_mesh(self, mesh: Mesh3D, model: Matrix4, color: Tuple[int, int, int] = (255, 255, 255)) -> None:
        tri_count = len(mesh.triangles)
        logger.info("[render] Drawing mesh with %d triangles", tri_count, tag="render")
        if tri_count > 50_000:
            logger.warn("[render] Mesh overflow: %d triangles", tri_count, tag="render")
            return False
        cam_api = core_get("camera3d")
        if cam_api is None:
            from ...camera import runtime as _cam_rt  # noqa: F401
            cam_api = core_get("camera3d")
        camera = cam_api.get("get_active_camera")() if cam_api else None  # type: ignore[assignment]
        if camera is None:
            logger.error("[core][render] No active camera found for frame render.")
            render_stats.stats["camera_missing_count"] += 1
            return False
        start = time.perf_counter()
        aspect = self.runtime.width / self.runtime.height if self.runtime.height else 1.0
        mvp = camera.get_projection_matrix(aspect) @ camera.get_view_matrix() @ model
        w, h = self.runtime.width, self.runtime.height
        line_col = (*color, 255)
        for tri in mesh.triangles:
            v0 = mvp.transform(mesh.vertices[tri[0]])
            v1 = mvp.transform(mesh.vertices[tri[1]])
            v2 = mvp.transform(mesh.vertices[tri[2]])
            pts = []
            for v in (v0, v1, v2):
                x = int((v.x * 0.5 + 0.5) * w)
                y = int((1.0 - (v.y * 0.5 + 0.5)) * h)
                pts.append((x, y, v.z))
            # wireframe overlay
            self.runtime.draw_line(pts[0][0], pts[0][1], pts[1][0], pts[1][1], line_col)
            self.runtime.draw_line(pts[1][0], pts[1][1], pts[2][0], pts[2][1], line_col)
            self.runtime.draw_line(pts[2][0], pts[2][1], pts[0][0], pts[0][1], line_col)
            self._rasterize_triangle(pts[0], pts[1], pts[2], color)
            render_stats.stats["triangles_drawn"] += 1
        render_stats.stats["frame3d_time"] += (time.perf_counter() - start) * 1000.0
        return True

    def _rasterize_triangle(self, v0, v1, v2, color) -> None:
        x0, y0, z0 = v0
        x1, y1, z1 = v1
        x2, y2, z2 = v2
        min_x = max(min(x0, x1, x2), 0)
        max_x = min(max(x0, x1, x2), self.runtime.width - 1)
        min_y = max(min(y0, y1, y2), 0)
        max_y = min(max(y0, y1, y2), self.runtime.height - 1)
        denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2)
        if denom == 0:
            return
        r, g, b = color
        for y in range(min_y, max_y + 1):
            for x in range(min_x, max_x + 1):
                w0 = (y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)
                w1 = (y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)
                w2 = denom - w0 - w1
                if w0 >= 0 and w1 >= 0 and w2 >= 0:
                    w0 /= denom
                    w1 /= denom
                    w2 /= denom
                    depth = z0 * w0 + z1 * w1 + z2 * w2
                    if self.zbuffer.test_and_set(x, y, depth):
                        self.runtime._blend_pixel(x, y, r, g, b, 255)
                        render_stats.stats["zbuffer_hits"] += 1
