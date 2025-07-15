
import json
import os
from .core.scenes.scene import Scene
from .core.engine import Engine

__all__ = ["save_game", "load_game"]


def save_game(engine: Engine, path: str) -> None:
    """Serialize the current engine state to ``path``."""
    scene = engine.scene
    data = {
        "scene": scene.to_dict() if scene is not None else {},
        "metadata": engine.metadata,
    }
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)


def load_game(engine: Engine, path: str) -> None:
    """Load engine state from ``path`` into ``engine``."""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    scene_data = data.get("scene", {})
    engine.scene = Scene.from_dict(scene_data, base_path=os.path.dirname(path))
    engine.scene_manager.add_scene("main", engine.scene)
    engine.scene_manager.set_active("main")
    engine.events = engine.scene.build_event_system(aggregate=False)
    engine.metadata.update(data.get("metadata", {}))
