__version__ = "0.1.0"

from engine.entities.object import register_role
from sage_adaptors.audio import register as register_audio


def _blink(obj, dt):
    t = getattr(obj, "_blink_t", 0.0) + dt
    if t >= 0.5:
        obj.visible = not getattr(obj, "visible", True)
        t = 0.0
    obj._blink_t = t


def init_engine(engine):
    register_role("blinking_sprite", logic=[_blink])
    register_audio()


