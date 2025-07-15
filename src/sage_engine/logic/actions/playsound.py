from ..base import Action, register_action, resolve_value
from ...utils.log import logger

try:
    from ... import play_sound
except Exception:  # pragma: no cover - optional dependency may be missing
    play_sound = None


@register_action('PlaySound', [('name', 'value')])
class PlaySound(Action):
    """Play a sound file via :class:`AudioManager`."""

    def __init__(self, name: str):
        self.name = name

    def execute(self, engine, scene, dt):
        if play_sound is None:
            logger.warning("Audio support unavailable; install miniaudio")
            return
        path = resolve_value(self.name, engine)
        try:
            play_sound(path)
        except Exception:
            logger.exception('Failed to play sound %s', path)
