"""Entry point for running the reference editor package."""

from __future__ import annotations

import sys
from pathlib import Path
from importlib import import_module

if __package__ is None:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

try:  # support execution via ``python path/to/__main__.py``
    from . import main
except Exception:  # pragma: no cover - fallback if package not initialised
    main = import_module("sage_editor").main

if __name__ == "__main__":  # pragma: no cover - CLI entry
    raise SystemExit(main())

