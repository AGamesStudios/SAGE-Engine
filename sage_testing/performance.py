"""Simple performance checks using profiling data."""
from __future__ import annotations

from typing import Dict

from sage_engine import profiling


def run_performance_checks() -> Dict[str, float]:
    return {
        "events_dropped": profiling.profile.events_dropped,
        "timers_dropped": profiling.profile.timers_dropped,
    }
