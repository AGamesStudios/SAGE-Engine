import math

from sage_engine.transform import (
    NodeTransform,
    Rect,
    Coord,
    Space,
    Camera2D,
    prepare_world_all,
    collect_visible,
    serialize_transform,
    apply_transform,
    pixel_snap,
    snap_rect,
    screen_rect_to_world,
    get_screen_bounds,
    intersects_screen,
)
from sage_engine.gfx.runtime import GraphicRuntime


def test_hierarchy_world_matrix():
    root = NodeTransform()
    root.transform.set_pos(10, 0)
    child = NodeTransform()
    root.add_child(child)
    child.transform.set_pos(5, 0)

    prepare_world_all(root)
    m_child = child.transform._m_world
    assert math.isclose(m_child[2], 15.0)


def test_dirty_flag_recomputes_once():
    node = NodeTransform()
    node.transform.set_pos(1, 2)
    prepare_world_all(node)
    first = node.transform._m_world[:]
    node.transform.set_pos(3, 4)
    prepare_world_all(node)
    second = node.transform._m_world
    assert first != second


def test_world_aabb_and_culling():
    root = NodeTransform(local_rect=Rect(0, 0, 10, 10))
    child = NodeTransform(local_rect=Rect(0, 0, 10, 10))
    child.transform.set_pos(100, 0)
    root.add_child(child)
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(50, 50))
    prepare_world_all(root)
    visible = collect_visible(root, cam)
    assert root in visible and child not in visible


def test_screen_bounds_and_intersects():
    root = NodeTransform(local_rect=Rect(0, 0, 10, 10))
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(20, 20))
    prepare_world_all(root)
    sb = get_screen_bounds(root, cam)
    assert math.isclose(sb.w, 10)
    assert intersects_screen(root, cam)
    cam.pos = (100, 100)
    assert not intersects_screen(root, cam)


def test_pixel_snap_and_screen_rect():
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(100, 100))
    rect = screen_rect_to_world(cam, Rect(0, 0, 100, 100, Space.SCREEN))
    assert math.isclose(rect.w, 100) and math.isclose(rect.h, 100)
    snapped = pixel_snap(Coord(0.25, 0.25, Space.WORLD), zoom=2.0)
    assert math.isclose(snapped.x, 0.5) and math.isclose(snapped.y, 0.5)
    rect2 = snap_rect(Rect(0.25, 0.25, 0.5, 0.5, Space.WORLD), zoom=2.0)
    assert math.isclose(rect2.x, 0.5) and math.isclose(rect2.w, 0.5)


def test_serialize_roundtrip():
    node = NodeTransform()
    node.transform.set_pos(3, 4)
    prepare_world_all(node)
    data = serialize_transform(node)
    other = NodeTransform()
    apply_transform(other, data)
    prepare_world_all(other)
    assert other.transform._m_world == node.transform._m_world


def test_transform_stats_reset():
    from sage_engine.transform import stats as tstats

    tstats.stats["nodes_updated"] = 5
    gfx = GraphicRuntime()
    gfx.begin_frame()
    assert tstats.stats["nodes_updated"] == 0


def test_global_helpers_and_visibility():
    root = NodeTransform(local_rect=Rect(0, 0, 10, 10))
    root.transform.set_pos(10, 0)
    child = NodeTransform(local_rect=Rect(0, 0, 5, 5))
    child.transform.set_pos(5, 0)
    root.add_child(child)
    child.transform.set_scale(2, 3)
    prepare_world_all(root)
    assert child.global_position() == (15, 0)
    sx, sy = child.global_scale()
    assert math.isclose(sx, 2.0) and math.isclose(sy, 3.0)
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(50, 50))
    assert child.is_visible(cam)
