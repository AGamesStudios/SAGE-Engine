from sage_engine.input import Input


def test_action_mapping():
    Input.map_action('jump', key='SPACE')
    Input.poll()
    Input._handle_key('SPACE', True)
    assert Input.is_action('jump')
