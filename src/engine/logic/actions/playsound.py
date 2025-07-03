from ..base import Action, register_action, resolve_value
from ...utils.log import logger

try:
    from ...audio import AudioManager
except Exception:  # pragma: no cover - optional dependency may be missing
    AudioManager = None


@register_action('PlaySound', [('name', 'value')])
class PlaySound(Action):
    """Play a sound file via :class:`AudioManager`."""

    def __init__(self, name: str):
        self.name = name

    def execute(self, engine, scene, dt):
        if AudioManager is None:
            logger.warning('AudioManager unavailable; install pygame')
            return
        am = getattr(engine, '_audio_manager', None)
        if am is None:
            try:
                am = AudioManager()
            except Exception:
                logger.exception('Failed to initialise audio')
                return
            engine._audio_manager = am
        path = resolve_value(self.name, engine)
        try:
            am.play(path)
        except Exception:
            logger.exception('Failed to play sound %s', path)
