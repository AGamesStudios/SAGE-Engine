import time
from sage_engine import core
from sage_engine.render import stats as render_stats

def _update():
    time.sleep(0.001)

core.register("update", _update)
core.register("draw", lambda: None)
core.register("flush", lambda: None)

def test_stats_have_phase_times():
    core.core_boot()
    core.core_tick()
    s = render_stats.stats
    assert s["ms_update"] >= 0
    assert s["ms_frame"] >= s["ms_update"]
    core.core_shutdown()
