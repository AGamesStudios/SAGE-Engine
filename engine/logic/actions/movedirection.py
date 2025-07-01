from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

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


