from sage_engine import core, time, timers, events


def setup_module():
    core.register('boot', time.boot)
    core.register('update', time.update)
    core.register('update', timers.update)
    core.register('flush', events.flush)


def test_core_cycle():
    core.core_boot({})
    core.core_tick()
    core.core_shutdown()
