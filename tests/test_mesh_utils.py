import math
import importlib
import pytest
spec = importlib.util.find_spec("engine.mesh_utils")
if not spec or spec.loader is None:
    pytest.skip("engine.mesh_utils unavailable", allow_module_level=True)
import engine.mesh_utils  # noqa: E402
importlib.reload(engine.mesh_utils)
from engine.mesh_utils import create_square_mesh, create_triangle_mesh  # noqa: E402


def test_square_mesh_center():
    mesh = create_square_mesh()
    xs = [v[0] for v in mesh.vertices]
    ys = [v[1] for v in mesh.vertices]
    assert min(xs) == -0.5
    assert max(xs) == 0.5
    assert min(ys) == -0.5
    assert max(ys) == 0.5


def test_triangle_mesh_centered():
    mesh = create_triangle_mesh()
    xs = [v[0] for v in mesh.vertices]
    ys = [v[1] for v in mesh.vertices]
    cx = (max(xs) + min(xs)) / 2
    cy = (max(ys) + min(ys)) / 2
    assert abs(cx) < 1e-6
    assert abs(cy) < 1e-6
    # sides have equal length
    verts = mesh.vertices
    d0 = math.dist(verts[0], verts[1])
    d1 = math.dist(verts[1], verts[2])
    d2 = math.dist(verts[2], verts[0])
    assert abs(d0 - d1) < 1e-6
    assert abs(d1 - d2) < 1e-6
