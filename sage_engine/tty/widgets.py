from __future__ import annotations


class Widget:
    def draw(self, tty, theme) -> None:  # pragma: no cover - interface
        raise NotImplementedError
