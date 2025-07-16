from sage_engine import coord


def test_coord_structs():
    w = coord.WorldSpace(5.0, 5.0)
    px, py = w.to_screen()
    back = coord.WorldSpace.from_screen(px, py)
    assert (back.x, back.y) == (5.0, 5.0)
    s = coord.ScreenSpace(px, py)
    wx, wy = s.to_world()
    assert (wx, wy) == (5.0, 5.0)
