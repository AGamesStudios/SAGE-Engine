from sage_engine.window import boot, reset, get_window, is_initialized
from sage.events import on


def test_window_events():
    events = []
    on("window_create", lambda data: events.append("create"))
    on("window_resize", lambda data: events.append((data["width"], data["height"])))
    on("window_close", lambda _: events.append("close"))
    boot()
    assert is_initialized()
    win = get_window()
    assert "create" in events
    win.resize(640, 480)
    assert (640, 480) in events
    reset()
    assert "close" in events
