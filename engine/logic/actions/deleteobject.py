from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('DeleteObject', [('target', 'object')])
class DeleteObject(Action):
    """Remove an object from the scene."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if scene:
            scene.remove_object(self.target)


