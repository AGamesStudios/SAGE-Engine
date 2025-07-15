class EngineExtension:
    """Base class for engine extensions."""

    def start(self, engine) -> None:
        """Called when the extension is added."""
        pass

    def update(self, engine, dt: float) -> None:
        """Called every frame with ``dt`` seconds."""
        pass

    def stop(self, engine) -> None:
        """Called when the engine shuts down or the extension is removed."""
        pass

__all__ = ["EngineExtension"]
