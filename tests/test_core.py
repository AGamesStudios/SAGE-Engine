from sage_engine import core, events
from sage_engine.scheduler import time, timers


def setup_module():
    core.register('boot', time.boot)
    core.register('update', time.update)
    core.register('update', timers.update)
    core.register('flush', events.flush)


def test_core_cycle():
    core.core_boot({})
    core.core_tick()
    core.core_shutdown()
