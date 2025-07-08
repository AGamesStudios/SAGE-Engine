import math
import importlib
import pytest
spec = importlib.util.find_spec("engine.mesh_utils")
if not spec or spec.loader is None:
    pytest.skip("engine.mesh_utils unavailable", allow_module_level=True)
import engine.mesh_utils  # noqa: E402
importlib.reload(engine.mesh_utils)
from engine.mesh_utils import (  # noqa: E402
    create_square_mesh,
    create_triangle_mesh,
    create_polygon_mesh,
    union_meshes,
)


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


def test_polygon_mesh_and_editing():
    mesh = create_polygon_mesh([(0, 0), (1, 0), (1, 1), (0, 1)])
    assert len(mesh.indices) == 6
    mesh.translate(1, 2)
    mesh.scale(2)
    mesh.rotate(math.pi / 2)
    xs = [v[0] for v in mesh.vertices]
    ys = [v[1] for v in mesh.vertices]
    assert len(xs) == 4 and len(ys) == 4


def test_union_meshes():
    square = create_square_mesh()
    tri = create_triangle_mesh()
    combo = union_meshes([square, tri])
    assert len(combo.vertices) == len(square.vertices) + len(tri.vertices)
    assert combo.indices is not None
    assert max(combo.indices) < len(combo.vertices)
