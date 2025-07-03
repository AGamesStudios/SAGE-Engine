from dataclasses import dataclass, field
from typing import Dict, Optional, TYPE_CHECKING

if TYPE_CHECKING:  # pragma: no cover - avoid heavy imports at runtime
    from .scene import Scene

@dataclass
class SceneManager:
    """Manage multiple scenes and expose the active one."""
    scenes: Dict[str, 'Scene'] = field(default_factory=dict)
    active: str | None = None

    def add_scene(self, name: str, scene: 'Scene') -> None:
        self.scenes[name] = scene
        if self.active is None:
            self.active = name

    def remove_scene(self, name: str) -> None:
        if name in self.scenes:
            del self.scenes[name]
            if self.active == name:
                self.active = next(iter(self.scenes), None)

    def set_active(self, name: str) -> None:
        if name in self.scenes:
            self.active = name

    def get_active_scene(self) -> Optional['Scene']:
        if self.active is not None:
            return self.scenes.get(self.active)
        return None

    def update(self, dt: float) -> None:
        scene = self.get_active_scene()
        if scene is not None:
            scene.update(dt)

    def draw(self, renderer) -> None:
        scene = self.get_active_scene()
        if scene is not None:
            scene.draw(renderer)
