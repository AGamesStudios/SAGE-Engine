"""Performance checks based on profiling stats."""
from __future__ import annotations

from typing import Dict

from .. import profiling


def run_performance_checks() -> Dict[str, float]:
    return {
        "events_dropped": profiling.profile.events_dropped,
        "timers_dropped": profiling.profile.timers_dropped,
    }
