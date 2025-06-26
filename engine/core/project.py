import json
from dataclasses import dataclass, field
import os

@dataclass(slots=True)
class Project:
    """Simple container for a SAGE project including scene data."""
    scene: dict
    renderer: str = "opengl"
    width: int = 640
    height: int = 480
    keep_aspect: bool = True
    background: tuple[int, int, int] = (0, 0, 0)
    title: str = 'SAGE 2D'
    version: str = '0.1.0'
    resources: str = 'resources'
    scenes: str = 'Scenes'
    scene_file: str = 'Scenes/Scene1.sagescene'
    metadata: dict = field(default_factory=dict)

    @classmethod
    def load(cls, path: str) -> "Project":
        with open(path, "r") as f:
            data = json.load(f)
        renderer = data.get("renderer", "opengl")
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
        keep_aspect = data.get('keep_aspect', True)
        background = tuple(data.get('background', (0, 0, 0)))
        title = data.get('title', 'SAGE 2D')
        version = data.get('version', '0.1.0')
        resources = data.get('resources', 'resources')
        metadata = data.get('metadata', {})
        return cls(scene or {}, renderer, width, height, keep_aspect,
                   background, title, version, resources, scenes_dir,
                   scene_file or 'Scenes/Scene1.sagescene', metadata)

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
                'keep_aspect': self.keep_aspect,
                'background': list(self.background),
                'title': self.title,
                'version': self.version,
                'resources': self.resources,
                'renderer': self.renderer,
                'scenes': self.scenes,
                'scene_file': self.scene_file,
                'metadata': self.metadata,
            }, f, indent=2)

