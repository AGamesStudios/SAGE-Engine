"""Local-import convenience for tests."""
from importlib import import_module
import sys

# Re-export the real package so ``from sage_engine import gui`` works when
# running from the repository root.  This mirrors ``src.sage_engine``.
_real = import_module('src.sage_engine')
globals().update(_real.__dict__)
sys.modules[__name__] = _real
