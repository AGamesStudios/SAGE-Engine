"""Select platform-specific window implementation."""

from __future__ import annotations

import sys

if sys.platform.startswith("win"):
    from .win32 import Window, create_window, process_events  # noqa: F401
else:
    from .stub import Window, create_window, process_events  # noqa: F401
