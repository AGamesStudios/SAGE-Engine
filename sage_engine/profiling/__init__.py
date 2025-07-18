"""Profiling helpers."""
from dataclasses import dataclass


@dataclass
class ProfileEntry:
    name: str
    ms: float


@dataclass
class ProfileFrame:
    entries: list[ProfileEntry]


__all__ = ["ProfileEntry", "ProfileFrame"]

