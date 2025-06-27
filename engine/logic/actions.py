"""Placeholder module for SAGE Logic actions.

No built-in actions are provided. Games and plugins should define their own
:class:`Action` subclasses and register them with :func:`register_action`.
"""

from .base import Action, register_action  # re-export for convenience

__all__ = []
