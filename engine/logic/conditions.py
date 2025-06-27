"""Placeholder module for SAGE Logic conditions.

No built-in conditions are provided. Games and plugins should define their own
:class:`Condition` subclasses and register them with :func:`register_condition`.
"""

from .base import Condition, register_condition  # re-export for convenience

__all__ = []
