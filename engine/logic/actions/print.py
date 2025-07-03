from ..base import Action, register_action, resolve_value
from ...utils.log import logger

@register_action('Print', [('text', 'value')])
class Print(Action):
    """Write text to the engine log."""

    def __init__(self, text):
        self.text = text

    def execute(self, engine, scene, dt):
        value = resolve_value(self.text, engine)
        if value is None:
            return
        if not isinstance(value, str):
            value = str(value)
        try:
            msg = value.format(**getattr(engine.events, 'variables', {}))
        except Exception:
            msg = value
        logger.info(msg)


