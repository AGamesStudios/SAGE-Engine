from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetCameraSize', [('target', 'object'), ('width', 'value'), ('height', 'value')])
class SetCameraSize(Action):
    """Change camera viewport size."""

    def __init__(self, target, width, height):
        self.target = target
        self.width = width
        self.height = height

    def execute(self, engine, scene, dt):
        self.target.width = int(resolve_value(self.width, engine))
        self.target.height = int(resolve_value(self.height, engine))


