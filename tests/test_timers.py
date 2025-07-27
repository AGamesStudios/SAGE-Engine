from sage_engine import profiling
from sage_engine.scheduler import time, timers


def test_timer_fires():
    time.boot({})
    fired = []

    def cb():
        fired.append(True)

    timers.manager.set(0.0, cb)
    timers.update()
    assert fired


def test_timer_overflow():
    timers.reset()
    time.boot({})
    counter = []

    def cb():
        counter.append(1)

    for _ in range(timers.manager.MAX_PER_FRAME + 5):
        timers.manager.set(0.0, cb)
    timers.update()
    assert len(counter) == timers.manager.MAX_PER_FRAME
    assert profiling.profile.timers_dropped > 0
