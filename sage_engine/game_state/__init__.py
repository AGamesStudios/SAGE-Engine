"""Simple game state manager."""

from __future__ import annotations

from typing import List


class GameState:
    def __init__(self) -> None:
        self.stack: List[str] = []

    def set_state(self, name: str) -> None:
        self.stack = [name]

    def push_state(self, name: str) -> None:
        self.stack.append(name)

    def pop_state(self) -> None:
        if self.stack:
            self.stack.pop()

    @property
    def current(self) -> str | None:
        return self.stack[-1] if self.stack else None


game_state = GameState()
