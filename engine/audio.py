"""Simple audio playback using pygame."""

from __future__ import annotations

import os
from typing import Dict

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

    def load_sound(self, name: str) -> pygame.mixer.Sound:
        """Load a sound file or ``.sageaudio`` descriptor."""
        path = get_resource_path(name)
        if path.endswith(".sageaudio"):
            from .formats import load_sageaudio

            meta = load_sageaudio(path)
            file_path = os.path.join(os.path.dirname(path), meta["file"])
        else:
            file_path = path
        try:
            sound = pygame.mixer.Sound(file_path)
        except Exception:
            logger.exception("Failed to load sound %s", file_path)
            raise
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

    def shutdown(self) -> None:
        for snd in self._sounds.values():
            snd.stop()
        self._sounds.clear()
        if pygame.mixer.get_init():
            pygame.mixer.quit()
