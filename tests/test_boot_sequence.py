from __future__ import annotations

import inspect

from sage_engine import core, window
from sage_engine.logger import log_crash


def test_boot_sequence(monkeypatch):
    monkeypatch.setenv("SAGE_HEADLESS", "1")
    window.init("t", 32, 32)
    core.core_boot()
    try:
        assert window.is_open()
        for _ in range(5):
            core.core_tick()
        # log_crash should handle non-traceback objects gracefully
        log_crash(RuntimeError, RuntimeError("x"), inspect.currentframe())
    finally:
        core.core_shutdown()

