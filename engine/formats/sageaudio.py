from __future__ import annotations

import json
import os
from typing import TypedDict


class SageAudio(TypedDict, total=False):
    file: str
    volume: float


def load_sageaudio(path: str) -> SageAudio:
    """Load a .sageaudio file and return its metadata."""
    with open(path, 'r', encoding='utf-8') as fh:
        data = json.load(fh)
    if 'file' not in data:
        raise ValueError('Missing audio file path')
    return data


def save_sageaudio(data: SageAudio, path: str) -> None:
    os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
    with open(path, 'w', encoding='utf-8') as fh:
        json.dump(data, fh, indent=2)
