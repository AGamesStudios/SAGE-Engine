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
    splitters: Dict[str, int] = field(default_factory=dict)


state = EditorState()


def load_state(path: str) -> None:
    """Load splitter positions from *path* if present."""
    import json
    from pathlib import Path

    p = Path(path)
    if not p.exists():
        return
    try:
        data = json.loads(p.read_text())
    except Exception:
        return
    state.splitters.update({k: int(v) for k, v in data.get("splitters", {}).items()})


def save_state(path: str) -> None:
    """Persist splitter positions to *path*."""
    import json
    from pathlib import Path

    p = Path(path)
    p.write_text(json.dumps({"splitters": state.splitters}))
