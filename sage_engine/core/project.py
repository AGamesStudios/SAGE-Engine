import json
from dataclasses import dataclass
import os

@dataclass
class Project:
    """Simple container for a SAGE project including scene data."""

    scene: dict
    renderer: str = 'opengl'

    @classmethod
    def load(cls, path: str) -> "Project":
        with open(path, 'r') as f:
            data = json.load(f)
        scene = data.get('scene')
        if isinstance(scene, str):
            # legacy format referencing a scene file
            if os.path.exists(scene):
                with open(scene, 'r') as sf:
                    scene = json.load(sf)
            else:
                scene = {}
        renderer = data.get('renderer', 'opengl')
        return cls(scene or {}, renderer)

    def save(self, path: str):
        with open(path, 'w') as f:
            json.dump({'scene': self.scene, 'renderer': self.renderer}, f, indent=2)

