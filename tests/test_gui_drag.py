from sage_engine.gui import drag, widgets, manager


def test_drag_events():
    w = widgets.Button()
    d = drag.DragController(w)
    events = []
    w.on_drag_start.connect(lambda x, y: events.append("start"))
    w.on_drag_move.connect(lambda x, y: events.append("move"))
    w.on_drag_end.connect(lambda x, y: events.append("end"))

    d.start(0, 0)
    d.move(1, 1)
    d.end(2, 2)
    assert events == ["start", "move", "end"]
