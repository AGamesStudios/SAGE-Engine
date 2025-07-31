"""Wrappers around SAGE engine functions used by the editor."""

from typing import Any


def load_scene(path: str) -> Any:
    """Load a scene into the engine."""
    # Placeholder implementation
    print(f"Loading scene: {path}")
    return None


def list_objects() -> list[str]:
    """Return a list of object identifiers in the current scene."""
    return []


def set_object_param(obj_id: str, category: str, param: str, value: Any) -> None:
    """Set parameter on an object through the engine."""
    print(f"Set {obj_id}.{category}.{param} = {value}")


def run_preview() -> None:
    """Run the current scene in preview mode."""
    print("Launching preview...")
