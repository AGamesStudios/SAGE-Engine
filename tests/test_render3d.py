from sage_engine.gfx.runtime import GraphicRuntime
from sage_engine.graphics import Mesh3D, Camera3D, Vector3, Matrix4
from sage_engine.render.units.render3d import Render3DUnit
from sage_engine.render import stats as render_stats, set_camera3d, draw_mesh
import sage_engine.gfx as gfx


def test_draw_simple_triangle():
    rt = GraphicRuntime()
    rt.init(64, 64)
    cam = Camera3D(position=Vector3(0, 0, -3), target=Vector3(0, 0, 0), aspect=1.0)
    unit = Render3DUnit(rt, cam)
    mesh = Mesh3D([Vector3(-1, -1, 0), Vector3(1, -1, 0), Vector3(0, 1, 0)], [(0, 1, 2)])
    render_stats.reset_frame()
    unit.draw_mesh(mesh, Matrix4.identity())
    assert render_stats.stats["triangles_drawn"] == 1
    assert render_stats.stats["zbuffer_hits"] > 0


def test_render_draw_mesh_wrapper():
    gfx.init(32, 32)
    gfx.begin_frame()
    cam = Camera3D(position=Vector3(0, 0, -3), target=Vector3(0, 0, 0), aspect=1.0)
    set_camera3d(cam)
    mesh = Mesh3D([Vector3(-1, -1, 0), Vector3(1, -1, 0), Vector3(0, 1, 0)], [(0, 1, 2)])
    render_stats.reset_frame()
    draw_mesh(mesh, Matrix4.identity())
    assert render_stats.stats["triangles_drawn"] == 1
