from __future__ import annotations

class GraphicState:
    def __init__(self) -> None:
        self.z = 0
        self.effects: list[str] = []

    def add_effect(self, name: str) -> None:
        if name not in self.effects:
            self.effects.append(name)

    def clear_effects(self) -> None:
        self.effects.clear()
