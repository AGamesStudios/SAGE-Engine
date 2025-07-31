"""Opens a preview window using the SAGE engine."""

from . import engine_api


def launch() -> None:
    """Run the current scene in a separate preview window."""
    engine_api.run_preview()
