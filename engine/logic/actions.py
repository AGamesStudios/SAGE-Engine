"""Placeholder module for SAGE Logic actions.

No built-in actions are provided. Games and plugins should define their own
:class:`Action` subclasses and register them with :func:`register_action`.
"""

from .base import Action, register_action, resolve_value  # re-export for convenience
from ..log import logger


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


__all__ = ['Print']
