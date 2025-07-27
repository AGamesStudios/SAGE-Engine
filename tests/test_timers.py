from sage_engine import time, timers


def test_timer_fires():
    time.boot({})
    fired = []

    def cb():
        fired.append(True)

    timers.manager.set(0.0, cb)
    timers.update()
    assert fired
