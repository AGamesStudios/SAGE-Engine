def test_window_move_and_close():
    from sage_engine.gui import widgets, manager
    win = widgets.Window(title="A", width=20, height=20)
    manager.root.add_child(win)
    win.start_drag(win.x + 1, win.y + 1)
    win.drag(10, 10)
    win.end_drag(10, 10)
    assert win.x == 9 and win.y == 9
    win.close()
    assert not win.visible


def test_popup_close():
    from sage_engine.gui import widgets
    p = widgets.Popup(width=10, height=10)
    p.close()
    assert not p.visible
