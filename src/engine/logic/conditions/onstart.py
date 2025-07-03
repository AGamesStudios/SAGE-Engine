from ..base import Condition, register_condition

@register_condition('OnStart', [])
class OnStart(Condition):
    """True on the first update after the event system is created or reset."""

    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

    def reset(self):
        self.triggered = False



