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

