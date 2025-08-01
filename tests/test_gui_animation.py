from sage_engine.gui import widgets, animation
from sage_engine.scheduler import timers, time


def test_animation_finish_callback():
    time.init()
    w = widgets.Button()
    flag = {"done": False}
    animation.animate(w, "x", 0, 10, 32, on_finish=lambda: flag.__setitem__("done", True))
    for _ in range(4):
        time.update()
        timers.update()
    assert w.x == 10
    assert flag["done"]
