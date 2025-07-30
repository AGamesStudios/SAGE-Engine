from sage_engine.input import Input


def test_input_mapping():
    Input.map_action('jump', key='SPACE')
    Input.poll()
    Input._handle_key('SPACE', True)
    assert Input.is_action('jump')
    Input.unmap_action('jump')
    assert not Input.is_action('jump')
