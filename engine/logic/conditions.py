"""Placeholder module for SAGE Logic conditions.

No built-in conditions are provided. Games and plugins should define their own
:class:`Condition` subclasses and register them with :func:`register_condition`.
"""

from .base import Condition, register_condition  # re-export for convenience


@register_condition('OnStart', [])
class OnStart(Condition):
    """True on the first update after the event system is created or reset."""

    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

    def reset(self):
        self.triggered = False


__all__ = ['OnStart']
