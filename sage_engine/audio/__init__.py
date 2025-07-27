"""Basic audio playback subsystem using miniaudio."""
from __future__ import annotations

import array
from typing import Optional

import miniaudio


class AudioHandle:
    """Handle to a playing sound."""

    def __init__(self, device: miniaudio.PlaybackDevice, generator):
        self.device = device
        self.generator = generator

    def stop(self) -> None:
        self.device.stop()
        self.device.close()


_backend: Optional[miniaudio.Backend] = None
_active: list[AudioHandle] = []


def set_backend(name: str) -> None:
    """Choose backend by name. Use 'null' for silent playback."""
    global _backend
    name = name.lower()
    if name == "null":
        _backend = miniaudio.Backend.NULL
    elif name == "alsa":
        _backend = miniaudio.Backend.ALSA
    elif name == "pulseaudio":
        _backend = miniaudio.Backend.PULSEAUDIO
    elif name == "wasapi":
        _backend = miniaudio.Backend.WASAPI
    else:
        _backend = None


def audio_boot() -> None:
    """Initialise the audio subsystem."""
    _active.clear()


def audio_shutdown() -> None:
    """Stop all playing sounds."""
    for h in list(_active):
        h.stop()
    _active.clear()


def _stream_file(path: str, volume: float, loop: bool):
    while True:
        stream = miniaudio.stream_file(path)
        frames = yield b""
        while True:
            chunk = stream.send(frames or 1024)
            if not chunk:
                break
            if volume != 1.0:
                for i in range(len(chunk)):
                    val = int(chunk[i] * volume)
                    if val > 32767:
                        val = 32767
                    elif val < -32768:
                        val = -32768
                    chunk[i] = val
            frames = yield chunk.tobytes()
        if not loop:
            break


def play(path: str, *, volume: float = 1.0, loop: bool = False) -> AudioHandle:
    """Play a sound file. Returns an AudioHandle."""
    gen = _stream_file(path, volume, loop)
    next(gen)
    if _backend is None:
        device = miniaudio.PlaybackDevice()
    else:
        device = miniaudio.PlaybackDevice(backends=[_backend])
    device.start(gen)
    handle = AudioHandle(device, gen)
    _active.append(handle)
    return handle


def stop(handle: AudioHandle) -> None:
    if handle in _active:
        handle.stop()
        _active.remove(handle)


boot = audio_boot
shutdown = audio_shutdown
__all__ = ["audio_boot", "audio_shutdown", "play", "stop", "set_backend", "AudioHandle", "boot", "shutdown"]
