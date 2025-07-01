from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetRotation', [('target', 'object'), ('angle', 'value')])
class SetRotation(Action):
    """Set object rotation in degrees."""

    def __init__(self, target, angle=0.0):
        self.target = target
        self.angle = angle

    def execute(self, engine, scene, dt):
        self.target.angle = float(resolve_value(self.angle, engine))


