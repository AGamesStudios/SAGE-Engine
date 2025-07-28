from sage_engine import window, events
import os


def test_init_shutdown():
    os.environ['SAGE_HEADLESS'] = '1'
    window.init("t", 320, 240)
    assert window.get_size() == (320, 240)
    window.shutdown()


def test_close_event():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init("t", 100, 100)
    # emulate close
    from sage_engine.window import _on_close
    _on_close()
    assert window.should_close()
    assert events.dispatcher.history[-1] == window.WIN_CLOSE
    window.shutdown()


def test_events_and_handle():
    os.environ['SAGE_HEADLESS'] = '1'
    events.reset()
    window.init('t', 50, 50)
    from sage_engine.window import _on_configure, _on_key, _on_mouse
    class E:
        def __init__(self, width=0, height=0, keysym='', keycode=0, type='move', x=0, y=0, button=0):
            self.width = width
            self.height = height
            self.keysym = keysym
            self.keycode = keycode
            self.type = type
            self.x = x
            self.y = y
            self.button = button

    _on_configure(E(width=40, height=30))
    _on_key(E(keysym='a', keycode=65))
    _on_mouse(E(type='down', x=5, y=6, button=1))
    assert window.get_size() == (40, 30)
    assert events.dispatcher.history[-3:] == [window.WIN_RESIZE, window.WIN_KEY, window.WIN_MOUSE]
    assert window.get_window_handle() is not None
    window.shutdown()

