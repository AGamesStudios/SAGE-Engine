from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetColor', [('target', 'object'), ('color', 'value')])
class SetColor(Action):
    """Change object RGB color."""

    def __init__(self, target, color):
        self.target = target
        self.color = color

    def execute(self, engine, scene, dt):
        value = resolve_value(self.color, engine)
        if isinstance(value, (list, tuple)):
            if len(value) == 3:
                value = (*value, 255)
            self.target.color = tuple(int(v) for v in value)


