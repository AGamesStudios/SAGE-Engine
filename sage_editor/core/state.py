"""Editor runtime state."""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Optional


@dataclass
class EditorState:
    scene_path: Optional[str] = None
    selected_object: Optional[int] = None
    panels: Dict[str, bool] = field(
        default_factory=lambda: {
            "World Manager": True,
            "Object View": True,
            "Role Editor": True,
            "Blueprint Designer": True,
            "Resource Manager": True,
        }
    )
