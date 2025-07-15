
import os

import json

from ..utils import load_json

__all__ = ["load_sagelogic", "save_sagelogic"]


def load_sagelogic(path: str) -> list[dict]:
    """Load event logic from ``path`` allowing comments."""
    data = load_json(path)
    events = data.get("events") if isinstance(data, dict) else data
    if not isinstance(events, list):
        raise ValueError("Logic file must contain an 'events' list or be a list itself")
    if any(not isinstance(evt, dict) for evt in events):
        raise ValueError("Event entries must be dictionaries")
    return events


def save_sagelogic(events: list[dict], path: str) -> None:
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "w", encoding="utf-8") as fh:
        json.dump({"events": events}, fh, indent=2)

