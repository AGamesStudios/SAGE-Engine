from sage_engine.input import Input


def test_key_states():
    Input.poll()
    Input._handle_key('A', True)
    assert Input.is_down('A')
    assert Input.is_pressed('A')
    Input.poll()
    Input._handle_key('A', False)
    assert Input.is_up('A')
    assert not Input.is_pressed('A')
    Input.poll()
