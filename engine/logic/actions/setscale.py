from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetScale', [('target', 'object'), ('x', 'value'), ('y', 'value')])
class SetScale(Action):
    """Set object scale."""

    def __init__(self, target, x=1.0, y=1.0):
        self.target = target
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.target.scale_x = float(resolve_value(self.x, engine))
        self.target.scale_y = float(resolve_value(self.y, engine))


