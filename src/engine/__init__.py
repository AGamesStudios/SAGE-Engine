# ruff: noqa
"""Temporary compatibility layer for ``import engine`` imports."""
import sys
import sage_engine as _mod

sys.modules[__name__] = _mod
