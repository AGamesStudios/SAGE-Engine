"""Role interfaces."""

from __future__ import annotations

from abc import ABC


class Role(ABC):
    """Base role behaviour."""

    def on_attach(self, obj: "Object") -> None:  # pragma: no cover - default
        pass

    def on_update(self, delta: float) -> None:  # pragma: no cover - default
        pass

    def on_render(self, ctx) -> None:  # pragma: no cover - default
        pass

    def on_event(self, evt) -> None:  # pragma: no cover - default
        pass
