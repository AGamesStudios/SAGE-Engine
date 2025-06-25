import json
from dataclasses import dataclass, field
import os
from .. import ENGINE_VERSION

@dataclass(slots=True)
class Project:
    """Simple container for a SAGE project including scene data."""

    scene: dict
    width: int = 640
    height: int = 480
    title: str = 'SAGE 2D'
    version: str = ENGINE_VERSION
    resources: str = 'resources'
    scenes: str = 'Scenes'
    scene_file: str = 'Scenes/Scene1.sagescene'
    metadata: dict = field(default_factory=dict)

    @classmethod
    def load(cls, path: str) -> "Project":
        with open(path, 'r') as f:
            data = json.load(f)
        scene = data.get('scene')
        scene_file = data.get('scene_file')
        scenes_dir = data.get('scenes', 'Scenes')
        if isinstance(scene, str):
            scene_file = scene_file or scene
        if scene_file:
            p = scene_file
            if not os.path.isabs(p):
                p = os.path.join(os.path.dirname(path), p)
            if os.path.exists(p):
                with open(p, 'r') as sf:
                    scene = json.load(sf)
            else:
                scene = {}
        width = data.get('width', 640)
        height = data.get('height', 480)
        title = data.get('title', 'SAGE 2D')
        version = data.get('version', ENGINE_VERSION)
        resources = data.get('resources', 'resources')
        metadata = data.get('metadata', {})
        return cls(scene or {}, width, height, title, version,
                   resources, scenes_dir, scene_file or 'Scenes/Scene1.sagescene',
                   metadata)

    def save(self, path: str):
        """Write the project file and associated scene."""
        if self.scene_file:
            scene_path = self.scene_file
            if not os.path.isabs(scene_path):
                scene_path = os.path.join(os.path.dirname(path), scene_path)
            os.makedirs(os.path.dirname(scene_path), exist_ok=True)
            with open(scene_path, 'w') as sf:
                json.dump(self.scene, sf, indent=2)
            scene_entry = self.scene_file
        else:
            scene_entry = self.scene
        with open(path, 'w') as f:
            json.dump({
                'scene': scene_entry,
                'width': self.width,
                'height': self.height,
                'title': self.title,
                'version': self.version,
                'resources': self.resources,
                'scenes': self.scenes,
                'scene_file': self.scene_file,
                'metadata': self.metadata,
            }, f, indent=2)

