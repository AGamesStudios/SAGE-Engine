"""Minimal 3D cube demo."""
from __future__ import annotations

import time

from sage_engine.gfx.runtime import GraphicRuntime
from sage_engine.graphics import Mesh3D, Camera3D, Vector3, Matrix4
from sage_engine.render.units.render3d import Render3DUnit
from sage_engine.window import init as win_init, is_open, get_window_handle
from sage_engine.runtime.fsync import FrameSync

win_init("3D Cube", 160, 120)
rt = GraphicRuntime()
rt.init(160, 120)
cam = Camera3D(position=Vector3(0, 0, -5), target=Vector3(0, 0, 0), aspect=160/120)
unit = Render3DUnit(rt, cam)

# cube mesh
verts = [
    Vector3(-1, -1, -1), Vector3(1, -1, -1), Vector3(1, 1, -1), Vector3(-1, 1, -1),
    Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(1, 1, 1), Vector3(-1, 1, 1)
]
tris = [
    (0,1,2),(0,2,3),(4,5,6),(4,6,7),(0,1,5),(0,5,4),(2,3,7),(2,7,6),(1,2,6),(1,6,5),(3,0,4),(3,4,7)
]
mesh = Mesh3D(verts, tris)
angle = 0.0
fsync = FrameSync()
while is_open():
    fsync.start_frame()
    rt.begin_frame((0, 0, 0, 255))
    rot = Matrix4.rotation_y(angle)
    unit.draw_mesh(mesh, rot)
    rt.flush_frame(get_window_handle(), fsync)
    fsync.end_frame()
    angle += 0.1
    time.sleep(0.001)
