"""Thin wrapper re-exporting the core Engine for compatibility."""
from sage_engine.core.engine import Engine, main

__all__ = ['Engine', 'main']
