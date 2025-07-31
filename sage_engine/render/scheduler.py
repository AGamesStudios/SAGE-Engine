from __future__ import annotations

"""Predictive frame scheduler to smooth spikes."""

from ctypes import c_void_p

from . import rustbridge as rust


class PredictiveScheduler:
    """Wrapper over Rust adaptive scheduler."""

    def __init__(self, history: int = 5) -> None:
        self._ptr: c_void_p = rust.lib.sage_sched_new(history)

    def __del__(self) -> None:
        if getattr(self, "_ptr", None):
            rust.lib.sage_sched_drop(self._ptr)

    def record(self, frame_time: float) -> None:
        rust.lib.sage_sched_record(self._ptr, float(frame_time))

    def should_defer(self, budget_ms: float) -> bool:
        return bool(rust.lib.sage_sched_should_defer(self._ptr, float(budget_ms)))
