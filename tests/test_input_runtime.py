from sage_engine.input import Input


def test_poll_cycle():
    Input._handle_key('Q', True)
    assert Input.is_down('Q')
    Input.poll()
    assert not Input.is_down('Q')
