from sage_engine import tasks
from sage_engine.scheduler import time


def test_task_schedule():
    time.boot({})
    called = []

    def cb():
        called.append(True)

    tasks.tasks.schedule(cb, delay_frames=1)
    time.update()
    tasks.update()
    assert not called
    time.update()
    tasks.update()
    assert called

def test_task_budget():
    tasks.reset()
    time.reset()
    called = []

    def cb():
        called.append(True)

    tasks.tasks.schedule(cb, delay_frames=0, budget=0.0)
    time.update()
    tasks.update()
    assert 'default' in tasks.tasks.heavy
    assert called
