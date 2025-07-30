from sage_engine.input import Input


def test_input_mapping():
    Input.map_action('jump', key='SPACE')
    Input.poll()
    Input._handle_key('SPACE', True)
    assert Input.is_action('jump')
    Input.unmap_action('jump')
    assert not Input.is_action('jump')


def test_key_mapping_space():
    Input.map_action('shoot', key='SPACE')
    Input.poll()
    Input._handle_key('SPACE', True)
    assert Input.is_action('shoot')
    Input.unmap_action('shoot')


def test_key_mapping_invalid():
    Input.map_action('foo', key='BLAHBLAH')
    Input._handle_key('BLAHBLAH', True)
    assert not Input.is_action('foo')
