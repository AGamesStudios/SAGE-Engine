def test_gui_animation_ease():
    from sage_engine.gui import widgets, animation
    from sage_engine.scheduler import timers, time

    time.boot({})
    w = widgets.Button()
    animation.animate(w, "x", 0, 5, 16, "ease-out")
    for _ in range(3):
        time.update()
        timers.update()
    assert w.x == 5
