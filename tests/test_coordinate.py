from sage_engine.transform import (
    Camera2D,
    Coord,
    NodeTransform,
    Space,
    local_to_world,
    screen_to_world,
    world_to_screen,
)


def test_world_screen_roundtrip():
    cam = Camera2D(pos=(0, 0), zoom=1.0, viewport_px=(100, 100))
    world = Coord(0, 0, Space.WORLD)
    screen = world_to_screen(cam, world)
    assert screen.space is Space.SCREEN
    back = screen_to_world(cam, screen)
    assert abs(back.x - world.x) < 1e-6
    assert abs(back.y - world.y) < 1e-6


def test_local_to_world():
    root = NodeTransform()
    root.transform.set_pos(1, 0)
    child = NodeTransform()
    root.add_child(child)
    child.transform.set_pos(2, 0)
    coord = local_to_world(child, Coord(0, 0, Space.LOCAL))
    assert coord.x == 3
