from sage_engine.input import Input


def test_mouse():
    Input.poll()
    Input._handle_mouse_move(10, 20)
    assert Input.mouse_position() == (10, 20)
    Input.poll()
    Input._handle_mouse_move(15, 25)
    dx, dy = Input.get_mouse_delta()
    assert (dx, dy) == (5, 5)
