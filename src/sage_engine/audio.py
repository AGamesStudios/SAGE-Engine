"""Audio playback via miniaudio."""
from __future__ import annotations

import array
import threading
from typing import Iterable

from .utils.log import logger

try:
    import miniaudio
    import lameenc
except Exception as exc:  # pragma: no cover - optional dependency
    miniaudio = None  # type: ignore
    lameenc = None  # type: ignore
    IMPORT_ERROR = exc
else:
    IMPORT_ERROR = None


class Sound:
    """Handle for a playing sound."""

    def __init__(self, path: str, loop: bool, gain: float, pan: float) -> None:
        if miniaudio is None:
            raise ImportError(
                "Audio features require miniaudio; install with 'pip install miniaudio'"
            ) from IMPORT_ERROR
        self.path = path
        self.loop = loop
        self.gain = gain
        self.pan = pan
        self.pitch = 1.0
        self._device: miniaudio.PlaybackDevice | None = None
        self._stop = threading.Event()

    def _stream(self) -> Iterable[bytes]:
        decoded = miniaudio.decode_file(self.path)
        frames = decoded.samples  # array.array
        channels = decoded.nchannels
        idx = 0
        frame_size = 1024 * channels
        while not self._stop.is_set():
            chunk = frames[idx : idx + frame_size]
            if not chunk:
                if self.loop:
                    idx = 0
                    continue
                break
            if self.gain != 1.0 or self.pan:
                data = array.array("h", chunk)
                for i in range(0, len(data), channels):
                    left = int(data[i] * self.gain)
                    if channels > 1:
                        right = int(data[i + 1] * self.gain)
                        if self.pan:
                            left = int(left * (1 - self.pan))
                            right = int(right * (1 + self.pan))
                        data[i] = left
                        data[i + 1] = right
                    else:
                        data[i] = left
                yield data.tobytes()
            else:
                yield array.array("h", chunk).tobytes()
            idx += frame_size

    def play(self) -> None:
        """Begin playback."""
        decoded = miniaudio.decode_file(self.path)
        rate = int(decoded.sample_rate * self.pitch)
        self._device = miniaudio.PlaybackDevice(
            output_format=miniaudio.SampleFormat.SIGNED16,
            nchannels=decoded.nchannels,
            sample_rate=rate,
            backends=[miniaudio.Backend.NULL],
        )
        gen = self._stream()
        next(gen, None)
        self._device.start(gen)

    def stop(self) -> None:
        if self._device:
            self._stop.set()
            try:
                self._device.stop()
                self._device.close()
            except Exception:  # pragma: no cover - cleanup best effort
                logger.debug("failed to stop audio")
            self._device = None

    def set_pitch(self, value: float) -> None:
        self.pitch = value
        if self._device:
            self.stop()
            self.play()


def play(path: str, *, loop: bool = False, gain: float = 1.0, pan: float = 0.0) -> Sound:
    """Play an audio file and return a :class:`Sound` handle."""
    snd = Sound(path, loop, gain, pan)
    snd.play()
    return snd
