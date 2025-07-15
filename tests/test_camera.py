from sage_engine import camera, coord


def test_camera_helpers():
    camera.set(10.0, 20.0, 2.0, dpi=1.0)
    px, py = coord.world_to_screen(20.0, 40.0)
    x, y = coord.screen_to_world(px, py)
    assert (x, y) == (20.0, 40.0)
    mat = camera.matrix()
    assert mat[0] == 2.0
    assert mat[2] == -20.0
    assert mat[5] == -40.0
