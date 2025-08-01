from sage_engine.scheduler import timers


def test_save_and_load(tmp_path):
    results = []
    timers.manager.set(0.0, lambda: results.append(1))
    state = timers.save()
    timers.reset()
    timers.load(state, lambda: results.append(2))
    timers.update()
    assert results == [2]
