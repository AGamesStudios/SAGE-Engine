from sage_engine.gui import animation
from sage_engine.scheduler import timers, time

class Dummy:
    x = 0

def test_animation():
    time.boot({})
    obj = Dummy()
    animation.animate(obj, "x", 0, 10, 32)
    for _ in range(5):
        time.update()
        timers.update()
    assert obj.x == 10
