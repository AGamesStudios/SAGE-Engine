from __future__ import annotations

import threading
import wave
from dataclasses import dataclass
from typing import Any

try:  # pragma: no cover - optional deps
    import simpleaudio
except ModuleNotFoundError:  # pragma: no cover - fallback stub
    simpleaudio = None

try:  # pragma: no cover - optional deps
    from pydub import AudioSegment
except ModuleNotFoundError:  # pragma: no cover - simple wave loader
    AudioSegment = None


@dataclass
class AudioHandle:
    path: str
    loop: bool = False
    pan: float = 0.0
    pitch: float = 1.0
    _playing: bool = False
    _thread: threading.Thread | None = None
    _wave_obj: Any = None
    _play_obj: Any = None

    def _load(self) -> None:
        if self._wave_obj is not None or simpleaudio is None:
            return
        if AudioSegment is not None:
            seg = AudioSegment.from_file(self.path)
            seg = seg.set_frame_rate(int(seg.frame_rate * self.pitch))
            raw = seg.raw_data
            self._wave_obj = simpleaudio.WaveObject(
                raw,
                num_channels=seg.channels,
                bytes_per_sample=seg.sample_width,
                sample_rate=seg.frame_rate,
            )
        else:
            with wave.open(self.path, "rb") as w:
                raw = w.readframes(w.getnframes())
            self._wave_obj = simpleaudio.WaveObject(
                raw,
                num_channels=1,
                bytes_per_sample=2,
                sample_rate=int(44100 * self.pitch),
            )

    def _play_loop(self) -> None:
        self._playing = True
        if self._wave_obj is None:
            self._load()
        while self._playing:
            if self._wave_obj:
                self._play_obj = self._wave_obj.play()
                self._play_obj.wait_done()
            if not self.loop:
                break
        self._playing = False

    def play(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._load()
        self._thread = threading.Thread(target=self._play_loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._playing = False
        if self._play_obj is not None:
            self._play_obj.stop()

    def is_playing(self) -> bool:
        return self._playing


class Mixer:
    """Very small mixer tracking active audio handles."""

    def __init__(self) -> None:
        self.master_volume: float = 1.0
        self._handles: list[AudioHandle] = []

    def register(self, handle: AudioHandle) -> None:
        self._handles.append(handle)

    def active(self) -> list[AudioHandle]:
        self._handles = [h for h in self._handles if h.is_playing()]
        return list(self._handles)


mixer = Mixer()


def play(path: str, *, loop: bool = False, pan: float = 0.0, pitch: float = 1.0) -> AudioHandle:
    """Play an audio file and return a handle."""
    handle = AudioHandle(path=path, loop=loop, pan=pan, pitch=pitch)
    handle.play()
    mixer.register(handle)
    return handle

__all__ = ["AudioHandle", "Mixer", "mixer", "play"]
