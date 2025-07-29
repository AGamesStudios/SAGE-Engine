from __future__ import annotations

import sys

from .base import LogHandler
from ..levels import DEBUG, INFO, WARN, ERROR, FATAL, CRITICAL

_COLORS = {
    DEBUG: "\033[36m",
    INFO: "\033[32m",
    WARN: "\033[33m",
    ERROR: "\033[31m",
    FATAL: "\033[35m",
    CRITICAL: "\033[41m",
}


class ConsoleHandler(LogHandler):
    def emit(self, record: str, level: int) -> None:
        if sys.stdout.isatty():
            color = _COLORS.get(level)
            if color:
                record = f"{color}{record}\033[0m"
        sys.stdout.write(record + "\n")
