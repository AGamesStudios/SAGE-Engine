"""Placeholder module for SAGE Logic actions.

No built-in actions are provided. Games and plugins should define their own
:class:`Action` subclasses and register them with :func:`register_action`.
"""

from .base import Action, register_action, resolve_value  # re-export for convenience
from ..utils.log import logger
import math
import operator


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


@register_action('SetVariable', [('name', 'variable'), ('value', 'value')])
class SetVariable(Action):
    """Assign ``value`` to a variable."""

    def __init__(self, name, value):
        self.name = name
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        engine.events.variables[self.name] = val


@register_action('ModifyVariable', [('name', 'variable'), ('op', 'value'), ('value', 'value')])
class ModifyVariable(Action):
    """Modify a numeric variable using ``op`` and ``value``."""

    def __init__(self, name, op='+', value=0):
        self.name = name
        self.op = op
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        current = engine.events.variables.get(self.name, 0)
        ops = {
            '+': operator.add,
            '-': operator.sub,
            '*': operator.mul,
            '/': operator.truediv,
        }
        func = ops.get(self.op, operator.add)
        try:
            engine.events.variables[self.name] = func(current, val)
        except Exception:
            logger.exception('ModifyVariable error')


@register_action('ShowObject', [('target', 'object')])
class ShowObject(Action):
    """Make an object visible by setting ``alpha`` to 1.0."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if hasattr(self.target, 'visible'):
            self.target.visible = True
        if hasattr(self.target, 'alpha'):
            self.target.alpha = 1.0


@register_action('HideObject', [('target', 'object')])
class HideObject(Action):
    """Hide an object by setting ``alpha`` to 0."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if hasattr(self.target, 'visible'):
            self.target.visible = False
        if hasattr(self.target, 'alpha'):
            self.target.alpha = 0.0


@register_action('SetPosition', [('target', 'object'), ('x', 'value'), ('y', 'value')])
class SetPosition(Action):
    """Set object position."""

    def __init__(self, target, x=0.0, y=0.0):
        self.target = target
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.target.x = float(resolve_value(self.x, engine))
        self.target.y = float(resolve_value(self.y, engine))


@register_action('SetRotation', [('target', 'object'), ('angle', 'value')])
class SetRotation(Action):
    """Set object rotation in degrees."""

    def __init__(self, target, angle=0.0):
        self.target = target
        self.angle = angle

    def execute(self, engine, scene, dt):
        self.target.angle = float(resolve_value(self.angle, engine))


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


@register_action('Flip', [('target', 'object'), ('flip_x', 'value'), ('flip_y', 'value')])
class Flip(Action):
    """Flip object horizontally or vertically."""

    def __init__(self, target, flip_x=False, flip_y=False):
        self.target = target
        self.flip_x = flip_x
        self.flip_y = flip_y

    def execute(self, engine, scene, dt):
        self.target.flip_x = bool(resolve_value(self.flip_x, engine))
        self.target.flip_y = bool(resolve_value(self.flip_y, engine))


@register_action('SetAlpha', [('target', 'object'), ('alpha', 'value')])
class SetAlpha(Action):
    """Set object alpha transparency (0..1)."""

    def __init__(self, target, alpha=1.0):
        self.target = target
        self.alpha = alpha

    def execute(self, engine, scene, dt):
        self.target.alpha = float(resolve_value(self.alpha, engine))


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


@register_action('SetZOrder', [('target', 'object'), ('z', 'value')])
class SetZOrder(Action):
    """Update the object's rendering order."""

    def __init__(self, target, z=0):
        self.target = target
        self.z = z

    def execute(self, engine, scene, dt):
        self.target.z = float(resolve_value(self.z, engine))
        if scene:
            scene.sort_objects()


@register_action('RenameObject', [('target', 'object'), ('name', 'value')])
class RenameObject(Action):
    """Rename the given object."""

    def __init__(self, target, name):
        self.target = target
        self.name = name

    def execute(self, engine, scene, dt):
        self.target.name = str(resolve_value(self.name, engine))


@register_action('DeleteObject', [('target', 'object')])
class DeleteObject(Action):
    """Remove an object from the scene."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if scene:
            scene.remove_object(self.target)


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


@register_action('SetCameraZoom', [('target', 'object'), ('zoom', 'value')])
class SetCameraZoom(Action):
    """Adjust camera zoom level."""

    def __init__(self, target, zoom=1.0):
        self.target = target
        self.zoom = zoom

    def execute(self, engine, scene, dt):
        self.target.zoom = float(resolve_value(self.zoom, engine))


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


@register_action('SetActiveCamera', [('camera', 'object')])
class SetActiveCamera(Action):
    """Make the given camera active."""

    def __init__(self, camera):
        self.camera = camera

    def execute(self, engine, scene, dt):
        if scene:
            scene.set_active_camera(self.camera)


@register_action('SetKeepAspect', [('value', 'value')])
class SetKeepAspect(Action):
    """Toggle renderer keep_aspect setting."""

    def __init__(self, value=True):
        self.value = value

    def execute(self, engine, scene, dt):
        val = bool(resolve_value(self.value, engine))
        if hasattr(engine.renderer, 'keep_aspect'):
            engine.renderer.keep_aspect = val



__all__ = [
    'Print', 'MoveDirection', 'SetVariable', 'ModifyVariable', 'ShowObject',
    'HideObject', 'SetPosition', 'SetRotation', 'SetScale', 'Flip', 'SetAlpha',
    'SetColor', 'SetSprite', 'SetZOrder', 'RenameObject', 'DeleteObject',
    'CreateObject', 'SetCameraZoom', 'SetCameraSize', 'SetActiveCamera',
    'SetKeepAspect'
]
