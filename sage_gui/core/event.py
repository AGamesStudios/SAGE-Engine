"""Event types used by SAGE GUI widgets."""

from enum import Enum, auto
from dataclasses import dataclass
from typing import Tuple


class EventType(Enum):
    MOUSE_MOVE = auto()
    MOUSE_DOWN = auto()
    MOUSE_UP = auto()
    KEY_DOWN = auto()
    KEY_UP = auto()


@dataclass
class Event:
    type: EventType
    position: Tuple[int, int] | None = None
    key: int | None = None
    button: int | None = None
