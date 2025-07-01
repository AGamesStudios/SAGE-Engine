from ..base import Action, register_action, resolve_value

@register_action('SetKeepAspect', [('value', 'value')])
class SetKeepAspect(Action):
    """Toggle renderer keep_aspect setting."""

    def __init__(self, value=True):
        self.value = value

    def execute(self, engine, scene, dt):
        val = bool(resolve_value(self.value, engine))
        if hasattr(engine.renderer, 'keep_aspect'):
            engine.renderer.keep_aspect = val
