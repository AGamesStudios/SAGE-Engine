from __future__ import annotations

import json
import os
import platform
import time
import traceback
from types import FrameType
from typing import Any

from .core import logger
from .errors import ERROR_CODES


def log_crash(exc_type: type, exc: BaseException, tb: FrameType | None, *, code: str = "SAGE_ERR_UNHANDLED", reason: str | None = None, module: str | None = None, phase: str | None = None, role: str | None = None) -> None:
    """Record crash information to file and console."""
    stack = traceback.format_exception(exc_type, exc, tb)
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    os.makedirs("logs", exist_ok=True)
    data = {
        "type": "crash",
        "code": code,
        "reason": reason or ERROR_CODES.get(code, str(exc)),
        "phase": phase or logger.phase_func(),
        "stack": stack,
        "module": module or getattr(exc, "__module__", ""),
        "role": role,
        "timestamp": timestamp,
        "system": {
            "platform": platform.system(),
            "python": platform.python_version(),
            "arch": platform.machine(),
        },
    }
    path = os.path.join("logs", f"crash_{timestamp}.json")
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=2)
    logger.error(f"CRITICAL CRASH DETECTED code={code} reason={data['reason']}", tag="crash", exc_info=False)
    for line in stack:
        logger.error(line.strip(), tag="crash")
