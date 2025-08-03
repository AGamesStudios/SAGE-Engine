import os

from sage_engine import core, window
from examples.sage_runner import main as runner


def test_runner_draws_sprite(monkeypatch):
    monkeypatch.setenv("SAGE_HEADLESS", "1")
    core.core_shutdown()
    core.register("boot", runner.boot)
    core.register("draw", runner.draw)
    core.core_boot()
    core.core_tick()
    stats = core.get("render").stats
    assert stats["sprites_drawn"] > 0
    assert window.get_framebuffer_size() == (640, 360)
    core.core_shutdown()

