from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('CreateObject', [('type', 'value'), ('x', 'value'), ('y', 'value'), ('z', 'value'), ('name', 'value')])
class CreateObject(Action):
    """Create a new object and add it to the scene."""

    def __init__(self, type='sprite', x=0.0, y=0.0, z=0.0, name=None):
        self.type = type
        self.x = x
        self.y = y
        self.z = z
        self.name = name

    def execute(self, engine, scene, dt):
        from ..core.objects import object_from_dict
        data = {
            'type': resolve_value(self.type, engine),
            'x': float(resolve_value(self.x, engine)),
            'y': float(resolve_value(self.y, engine)),
            'z': float(resolve_value(self.z, engine)),
        }
        if self.name is not None:
            data['name'] = resolve_value(self.name, engine)
        obj = object_from_dict(data)
        if obj and scene:
            scene.add_object(obj)


