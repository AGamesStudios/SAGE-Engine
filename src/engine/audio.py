"""Simple audio playback using pygame."""


import os
from typing import Dict, cast

from .core.resources import get_resource_path
from .utils.log import logger

try:
    import pygame
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "Audio features require pygame; install with 'pip install pygame'"
    ) from exc


class AudioManager:
    """Load and play sound effects using ``pygame.mixer``."""

    def __init__(self) -> None:
        os.environ.setdefault("SDL_AUDIODRIVER", "dummy")
        if not pygame.mixer.get_init():
            try:
                pygame.mixer.init()
            except Exception:
                logger.exception("Failed to initialise audio")
                raise
        self._sounds: Dict[str, pygame.mixer.Sound] = {}
        self._music_path: str | None = None

    def load_sound(self, name: str) -> pygame.mixer.Sound:
        """Load a sound file or ``.sageaudio`` descriptor."""
        path = get_resource_path(name)
        meta = None
        if path.endswith(".sageaudio"):
            from .formats import load_sageaudio

            meta = load_sageaudio(path)
            file_path = os.path.join(
                os.path.dirname(path), cast(str, meta["file"])  # pyright: ignore[reportTypedDictNotRequiredAccess]
            )
        else:
            file_path = path
        try:
            sound = pygame.mixer.Sound(file_path)
        except Exception:
            logger.exception("Failed to load sound %s", file_path)
            raise
        if meta and "volume" in meta:
            try:
                sound.set_volume(float(meta["volume"]))
            except Exception:
                logger.exception("Invalid volume value: %s", meta["volume"])
        self._sounds[name] = sound
        return sound

    def play(self, name: str) -> None:
        """Play a previously loaded sound or load it on demand."""
        sound = self._sounds.get(name)
        if sound is None:
            sound = self.load_sound(name)
        sound.play()

    def stop(self, name: str) -> None:
        sound = self._sounds.get(name)
        if sound:
            sound.stop()

    def load_music(self, path: str) -> None:
        """Load a music file."""
        path = get_resource_path(path)
        try:
            pygame.mixer.music.load(path)
            self._music_path = path
        except Exception:
            logger.exception("Failed to load music %s", path)
            raise

    def play_music(self, path: str | None = None, *, loops: int = 0) -> None:
        """Play music once loaded."""
        if path:
            self.load_music(path)
        if self._music_path:
            pygame.mixer.music.play(loops)

    def stop_music(self) -> None:
        pygame.mixer.music.stop()

    def pause_music(self) -> None:
        pygame.mixer.music.pause()

    def unpause_music(self) -> None:
        pygame.mixer.music.unpause()

    def set_music_volume(self, vol: float) -> None:
        try:
            pygame.mixer.music.set_volume(float(vol))
        except Exception:
            logger.exception("Invalid music volume: %s", vol)

    def shutdown(self) -> None:
        for snd in self._sounds.values():
            snd.stop()
        self._sounds.clear()
        pygame.mixer.music.stop()
        if pygame.mixer.get_init():
            pygame.mixer.quit()
