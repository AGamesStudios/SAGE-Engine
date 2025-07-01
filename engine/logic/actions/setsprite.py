from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetSprite', [('target', 'object'), ('path', 'value')])
class SetSprite(Action):
    """Change the object's sprite image."""

    def __init__(self, target, path):
        self.target = target
        self.path = path

    def execute(self, engine, scene, dt):
        p = resolve_value(self.path, engine)
        self.target.image_path = str(p)
        if hasattr(self.target, '_load_image'):
            try:
                self.target._load_image()
            except Exception:
                logger.exception('Failed to load sprite %s', p)


