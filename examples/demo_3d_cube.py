"""Demo showcasing basic 3D cube rendering."""
from __future__ import annotations

import time

import sage_engine.gfx as gfx
from sage_engine.graphics import Mesh3D, Vector3, Matrix4, Camera3D
from sage_engine.render import draw_mesh
from sage_engine.camera import runtime as cam_runtime
from sage_engine.window import init as win_init, is_open, get_window_handle

# initialize window and graphics
win_init("3D Cube", 800, 600)
gfx.init(800, 600)

# configure camera
camera = Camera3D(position=Vector3(0, 0, -5), look_at=Vector3(0, 0, 0))
cam_runtime.set_active_camera(camera)

# cube geometry
verts = [
    Vector3(-1, -1, -1), Vector3(1, -1, -1), Vector3(1, 1, -1), Vector3(-1, 1, -1),
    Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(1, 1, 1), Vector3(-1, 1, 1),
]
tris = [
    (0, 1, 2), (0, 2, 3),
    (4, 5, 6), (4, 6, 7),
    (0, 1, 5), (0, 5, 4),
    (2, 3, 7), (2, 7, 6),
    (1, 2, 6), (1, 6, 5),
    (3, 0, 4), (3, 4, 7),
]
mesh = Mesh3D(verts, tris)

angle = 0.0
while is_open():
    gfx.begin_frame((0, 0, 0, 255))
    rot = Matrix4.rotation_y(angle)
    draw_mesh(mesh, rot)
    gfx.flush_frame(get_window_handle())
    angle += 0.05
    time.sleep(1 / 60)
