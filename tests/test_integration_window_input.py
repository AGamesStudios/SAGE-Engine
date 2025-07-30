from sage_engine.input import Input, process_win32_key_event, WM_KEYDOWN, WM_KEYUP


def test_integration_window_input():
    Input.poll()
    process_win32_key_event(WM_KEYDOWN, 65, 0)  # 'A'
    assert Input.is_down('A') and Input.is_pressed('A')
    Input.poll()
    process_win32_key_event(WM_KEYUP, 65, 0)
    assert Input.is_up('A')
    Input.poll()
