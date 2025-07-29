from sage_engine.input import Input


def test_poll_clears_states():
    Input._handle_key('Q', True)
    assert Input.was_pressed('Q')
    Input.poll()
    assert not Input.was_pressed('Q')
