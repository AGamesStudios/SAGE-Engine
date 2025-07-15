from __future__ import annotations

import importlib
import time
from pathlib import Path
__all__ = ["check_reload", "watch_reload"]


def check_reload(flag: str | Path = "reload.flag") -> bool:
    """Reload nano_core if *flag* exists. Returns ``True`` on reload."""
    path = Path(flag)
    if not path.exists():
        return False
    path.unlink()
    try:
        module = importlib.import_module("nano_core")
    except ModuleNotFoundError:
        return False
    importlib.reload(module)
    return True


def watch_reload(flag: str | Path = "reload.flag", interval: float = 0.5) -> None:
    """Continuously watch for reload flag and reload nano_core."""
    while True:
        check_reload(flag)
        time.sleep(interval)
