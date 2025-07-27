from sage_engine.scheduler import time


def test_time_advances():
    time.boot({})
    t1 = time.get_time().time
    time.update()
    assert time.get_time().time >= t1
