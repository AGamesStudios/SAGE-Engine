"""Select platform-specific TTY implementation."""

from __future__ import annotations

import sys

if sys.platform.startswith("win"):
    from .win32 import setup_terminal, restore_terminal, read_key  # noqa: F401
elif sys.platform.startswith("linux") or sys.platform.startswith("darwin"):
    from .unix import setup_terminal, restore_terminal, read_key  # noqa: F401
else:
    from .dummy import setup_terminal, restore_terminal, read_key  # noqa: F401
