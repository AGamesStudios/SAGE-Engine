from sage_engine.input import Input, process_win32_key_event, WM_KEYDOWN


def test_map_space_action():
    Input.map_action('shoot', key='SPACE')
    Input.poll()
    process_win32_key_event(WM_KEYDOWN, 32, 0)
    assert Input.is_action('shoot')
    Input.unmap_action('shoot')
    Input.poll()
