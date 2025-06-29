"""Placeholder module for SAGE Logic actions.

No built-in actions are provided. Games and plugins should define their own
:class:`Action` subclasses and register them with :func:`register_action`.
"""

from .base import Action, register_action, resolve_value  # re-export for convenience
from ..log import logger
import math


@register_action('Print', [('text', 'value')])
class Print(Action):
    """Write text to the engine log."""

    def __init__(self, text):
        self.text = text

    def execute(self, engine, scene, dt):
        value = resolve_value(self.text, engine)
        if value is None:
            logger.info('')
            return
        if not isinstance(value, str):
            value = str(value)
        try:
            msg = value.format(**getattr(engine.events, 'variables', {}))
        except Exception:
            msg = value
        logger.info(msg)


@register_action('MoveDirection', [('target', 'object'),
                                   ('direction', 'value'),
                                   ('speed', 'value')])
class MoveDirection(Action):
    """Move an object along ``direction`` degrees using ``speed``."""

    def __init__(self, target, direction=0.0, speed=0.0):
        self.target = target
        self.direction = direction
        self.speed = speed

    def execute(self, engine, scene, dt):
        ang = resolve_value(self.direction, engine)
        spd = resolve_value(self.speed, engine)
        try:
            ang = float(ang)
            spd = float(spd)
        except Exception:
            logger.exception('Invalid MoveDirection values')
            return
        dx = math.cos(math.radians(ang)) * spd * dt
        dy = math.sin(math.radians(ang)) * spd * dt
        self.target.x += dx
        self.target.y += dy


__all__ = ['Print', 'MoveDirection']
