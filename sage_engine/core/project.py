import json
from dataclasses import dataclass

@dataclass
class Project:
    """Simple container for a SAGE project."""
    scene: str

    @classmethod
    def load(cls, path: str) -> "Project":
        with open(path, 'r') as f:
            data = json.load(f)
        return cls(data.get('scene', ''))

    def save(self, path: str):
        with open(path, 'w') as f:
            json.dump({'scene': self.scene}, f, indent=2)

