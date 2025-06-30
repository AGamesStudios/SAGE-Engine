from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Tuple, Optional


@dataclass
class SceneNode:
    """Represents a single scene inside a :class:`SceneGraph`."""

    name: str
    scene_file: str
    next_nodes: List[str] = field(default_factory=list)
    screenshot: Optional[str] = None
    position: Tuple[int, int] = (0, 0)


@dataclass
class SceneGraph:
    """Simple directed graph describing scene order for a project."""

    nodes: Dict[str, SceneNode] = field(default_factory=dict)
    start: Optional[str] = None
    metadata: dict = field(default_factory=dict)

    def add_scene(self, name: str, scene_file: str) -> SceneNode:
        """Add a new scene node to the graph."""
        if name in self.nodes:
            raise ValueError(f"Scene {name!r} already exists")
        node = SceneNode(name=name, scene_file=scene_file)
        self.nodes[name] = node
        if self.start is None:
            self.start = name
        return node

    def connect(self, src: str, dst: str) -> None:
        """Create a connection from ``src`` to ``dst``."""
        if src not in self.nodes or dst not in self.nodes:
            raise KeyError("Both nodes must exist to connect")
        if dst not in self.nodes[src].next_nodes:
            self.nodes[src].next_nodes.append(dst)

    @classmethod
    def from_dict(cls, data: dict) -> "SceneGraph":
        graph = cls(metadata=data.get("metadata", {}), start=data.get("start"))
        for name, entry in data.get("nodes", {}).items():
            node = SceneNode(
                name=name,
                scene_file=entry.get("scene_file", ""),
                next_nodes=list(entry.get("next", [])),
                screenshot=entry.get("screenshot"),
                position=tuple(entry.get("position", (0, 0))),
            )
            graph.nodes[name] = node
        return graph

    def to_dict(self) -> dict:
        return {
            "metadata": self.metadata,
            "start": self.start,
            "nodes": {
                name: {
                    "scene_file": node.scene_file,
                    "next": node.next_nodes,
                    "screenshot": node.screenshot,
                    "position": list(node.position),
                }
                for name, node in self.nodes.items()
            },
        }
