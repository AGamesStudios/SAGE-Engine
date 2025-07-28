"""Minimal audio API placeholder."""

from __future__ import annotations

from typing import Dict


class Audio:
    def __init__(self) -> None:
        self.volumes: Dict[str, float] = {"master": 1.0}

    def play(self, name: str) -> None:  # pragma: no cover - placeholder
        pass

    def set_volume(self, channel: str, value: float) -> None:
        self.volumes[channel] = value


audio = Audio()
