"""Demonstrates the InkEmitter using the spark preset."""
from __future__ import annotations

import json
import time
from pathlib import Path

from sage_engine.ink import InkEmitter


def load_preset(name: str) -> InkEmitter:
    path = Path(__file__).with_name(f"{name}.json")
    data = json.loads(path.read_text())
    return InkEmitter(
        rate=data.get("rate", 50),
        velocity_range=tuple(data.get("velocity_range", [-10, 10])),
        life_time=data.get("life_time", 1.0),
    )


def main() -> None:
    emitter = load_preset("spark")
    start = time.time()
    while time.time() - start < 1.0:
        emitter.emit(0.016)
        emitter.update(0.016)
        time.sleep(0.016)
    print(len(emitter.particles), "particles alive")


if __name__ == "__main__":
    main()
