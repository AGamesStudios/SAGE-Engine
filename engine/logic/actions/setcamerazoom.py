from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetCameraZoom', [('target', 'object'), ('zoom', 'value')])
class SetCameraZoom(Action):
    """Adjust camera zoom level."""

    def __init__(self, target, zoom=1.0):
        self.target = target
        self.zoom = zoom

    def execute(self, engine, scene, dt):
        self.target.zoom = float(resolve_value(self.zoom, engine))


