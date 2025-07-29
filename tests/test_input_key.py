from sage_engine.input import Input


def test_key_press_release():
    Input.poll()
    Input._handle_key('A', True)
    assert Input.was_pressed('A')
    assert Input.is_pressed('A')
    Input.poll()
    Input._handle_key('A', False)
    assert Input.was_released('A')
    assert not Input.is_pressed('A')
    Input.poll()
