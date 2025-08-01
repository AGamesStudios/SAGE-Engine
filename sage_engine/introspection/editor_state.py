from dataclasses import dataclass, field
from typing import Any, Dict


@dataclass
class EditorState:
    """Simple container for editor flags and temporary data."""

    flags: Dict[str, Any] = field(default_factory=dict)
