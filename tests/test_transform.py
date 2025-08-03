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
    screen_rect_to_world,
)


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


def test_pixel_snap_and_screen_rect():
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(100, 100))
    rect = screen_rect_to_world(cam, Rect(0, 0, 100, 100, Space.SCREEN))
    assert math.isclose(rect.w, 100) and math.isclose(rect.h, 100)
    snapped = pixel_snap(Coord(10.3, 9.7, Space.SCREEN))
    assert snapped.x == 10 and snapped.y == 10


def test_serialize_roundtrip():
    node = NodeTransform()
    node.transform.set_pos(3, 4)
    prepare_world_all(node)
    data = serialize_transform(node)
    other = NodeTransform()
    apply_transform(other, data)
    prepare_world_all(other)
    assert other.transform._m_world == node.transform._m_world
