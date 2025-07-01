from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

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
    'SetKeepAspect', 'SetObjectVariable', 'ModifyObjectVariable'
]
