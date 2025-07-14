import math
import importlib
import sys
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
    triangulate_mesh,
    union_meshes,
    difference_meshes,
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
    assert mesh.indices is None
    mesh.translate(1, 2)
    mesh.scale(2)
    mesh.rotate(math.pi / 2)
    assert len(mesh.vertices) == 4


def test_mesh_apply_matrix():
    from engine.core.math2d import make_transform

    mesh = create_square_mesh()
    mat = make_transform(1, 2)
    mesh.apply_matrix(mat)
    assert mesh.vertices[0] == pytest.approx((0.5, 1.5))
    assert mesh.vertices[2] == pytest.approx((1.5, 2.5))


def test_union_meshes():
    square = create_square_mesh()
    tri = create_triangle_mesh()
    combo = union_meshes([square, tri])
    assert len(combo.vertices) == len(square.vertices) + len(tri.vertices)
    assert combo.indices is not None
    assert max(combo.indices) < len(combo.vertices)


def test_union_meshes_multipolygon():
    pytest.importorskip("shapely")
    a = create_square_mesh()
    b = create_square_mesh()
    b.translate(5, 0)
    result = union_meshes([a, b], negatives=[create_triangle_mesh()])
    assert len(result.vertices) > len(a.vertices)
    assert result.indices is not None


def test_difference_meshes():
    pytest.importorskip("shapely")
    square = create_square_mesh()
    tri = create_triangle_mesh()
    union = union_meshes([square])
    result = difference_meshes(union, [tri])
    assert len(result.vertices) > 0
    assert result.indices is not None


def test_difference_requires_shapely(monkeypatch):
    monkeypatch.setitem(sys.modules, "shapely", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry.base", None)
    monkeypatch.setitem(sys.modules, "shapely.ops", None)
    import importlib
    import engine.mesh_utils as mu
    importlib.reload(mu)
    square = create_square_mesh()
    with pytest.raises(ImportError):
        mu.difference_meshes(square, [])


def test_union_requires_shapely(monkeypatch):
    monkeypatch.setitem(sys.modules, "shapely", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry.base", None)
    monkeypatch.setitem(sys.modules, "shapely.ops", None)
    import importlib
    import engine.mesh_utils as mu
    importlib.reload(mu)
    square = create_square_mesh()
    with pytest.raises(ImportError):
        mu.union_meshes([square], negatives=[square])


def test_concave_polygon():
    verts = [(0, 0), (2, 0), (2, 2), (1, 1), (0, 2)]
    mesh = create_polygon_mesh(verts)
    assert mesh.indices is None


def test_concave_polygon_no_shapely(monkeypatch):
    monkeypatch.setitem(sys.modules, "shapely", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry.base", None)
    monkeypatch.setitem(sys.modules, "shapely.ops", None)
    import importlib
    import engine.mesh_utils as mu
    importlib.reload(mu)
    verts = [(0, 0), (2, 0), (2, 2), (1, 1), (0, 2)]
    mesh = mu.create_polygon_mesh(verts)
    assert mesh.indices is None


def test_self_intersecting_polygon():
    verts = [(0, 0), (2, 2), (0, 2), (2, 0)]
    mesh = create_polygon_mesh(verts)
    assert mesh.indices is None


def test_polygon_with_collinear_points():
    verts = [(0, 0), (1, 0), (2, 0), (2, 1), (1, 2), (0, 1)]
    mesh = create_polygon_mesh(verts)
    assert mesh.indices is None


def test_triangulate_mesh():
    verts = [(0, 0), (2, 0), (2, 2), (1, 1), (0, 2)]
    poly = create_polygon_mesh(verts)
    tri = triangulate_mesh(poly)
    assert tri.indices is not None
    assert len(tri.indices) % 3 == 0


def test_triangulate_mesh_no_shapely(monkeypatch):
    monkeypatch.setitem(sys.modules, "shapely", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry", None)
    monkeypatch.setitem(sys.modules, "shapely.geometry.base", None)
    monkeypatch.setitem(sys.modules, "shapely.ops", None)
    import importlib
    import engine.mesh_utils as mu
    importlib.reload(mu)
    poly = mu.create_polygon_mesh([(0, 0), (2, 0), (2, 2), (1, 1), (0, 2)])
    tri = mu.triangulate_mesh(poly)
    assert tri.indices is not None

