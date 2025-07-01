from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Tuple, Optional


@dataclass
class BaseNode:
    """Generic node for :class:`BaseGraph`."""

    name: str
    next_nodes: List[str] = field(default_factory=list)
    position: Tuple[int, int] = (0, 0)
    metadata: dict = field(default_factory=dict)


class BaseGraph:
    """Simple directed graph container for arbitrary nodes."""

    def __init__(self, *, start: Optional[str] = None, metadata: Optional[dict] = None) -> None:
        self.nodes: Dict[str, BaseNode] = {}
        self.start: Optional[str] = start
        self.metadata: dict = metadata or {}

    def add_node(self, node: BaseNode) -> BaseNode:
        if node.name in self.nodes:
            raise ValueError(f"Node {node.name!r} already exists")
        self.nodes[node.name] = node
        if self.start is None:
            self.start = node.name
        return node

    def connect(self, src: str, dst: str) -> None:
        if src not in self.nodes or dst not in self.nodes:
            raise KeyError("Both nodes must exist to connect")
        if dst not in self.nodes[src].next_nodes:
            self.nodes[src].next_nodes.append(dst)

    @classmethod
    def from_dict(cls, data: dict) -> "BaseGraph":
        graph = cls(start=data.get("start"), metadata=data.get("metadata", {}))
        for name, entry in data.get("nodes", {}).items():
            node = BaseNode(
                name=name,
                next_nodes=list(entry.get("next", [])),
                position=tuple(entry.get("position", (0, 0))),
                metadata=dict(entry.get("metadata", {})),
            )
            graph.nodes[name] = node
        return graph

    def to_dict(self) -> dict:
        return {
            "metadata": self.metadata,
            "start": self.start,
            "nodes": {
                name: {
                    "next": node.next_nodes,
                    "position": list(node.position),
                    "metadata": node.metadata,
                }
                for name, node in self.nodes.items()
            },
        }


@dataclass
class SceneNode(BaseNode):
    """Node describing a scene and its connections."""

    scene_file: str = ""
    screenshot: Optional[str] = None


class SceneGraph(BaseGraph):
    """Directed graph describing scene order for a project."""

    def add_scene(
        self,
        name: str,
        scene_file: str,
        *,
        screenshot: Optional[str] = None,
        position: Tuple[int, int] = (0, 0),
        metadata: Optional[dict] = None,
    ) -> SceneNode:
        node = SceneNode(
            name=name,
            scene_file=scene_file,
            screenshot=screenshot,
            position=position,
            metadata=metadata or {},
        )
        return self.add_node(node)  # type: ignore[arg-type]

    @classmethod
    def from_dict(cls, data: dict) -> "SceneGraph":
        graph = cls(start=data.get("start"), metadata=data.get("metadata", {}))
        for name, entry in data.get("nodes", {}).items():
            node = SceneNode(
                name=name,
                scene_file=entry.get("scene_file", ""),
                next_nodes=list(entry.get("next", [])),
                screenshot=entry.get("screenshot"),
                position=tuple(entry.get("position", (0, 0))),
                metadata=dict(entry.get("metadata", {})),
            )
            graph.nodes[name] = node
        return graph

    def to_dict(self) -> dict:
        data = super().to_dict()
        for name, node in self.nodes.items():
            if isinstance(node, SceneNode):
                data["nodes"][name]["scene_file"] = node.scene_file
                if node.screenshot:
                    data["nodes"][name]["screenshot"] = node.screenshot
        return data
