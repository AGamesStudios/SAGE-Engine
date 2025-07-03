from ..base import Action, register_action, resolve_value
from ...utils.log import logger

@register_action('CallFunction', [('func', 'value'), ('args', 'value')])
class CallFunction(Action):
    """Call a Python function with optional arguments."""

    def __init__(self, func, args=None):
        self.func = func
        self.args = args if args is not None else []

    def execute(self, engine, scene, dt):
        func = resolve_value(self.func, engine)
        args = resolve_value(self.args, engine)
        if callable(func):
            try:
                if isinstance(args, list):
                    func(*args)
                else:
                    func(args)
            except Exception:
                logger.exception('CallFunction error')
