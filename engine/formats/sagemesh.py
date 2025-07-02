from __future__ import annotations

import json
import os

from ..mesh_utils import Mesh


def load_sagemesh(path: str) -> Mesh:
    """Load a mesh from a .sagemesh file."""
    with open(path, 'r', encoding='utf-8') as fh:
        data = json.load(fh)
    vertices = [tuple(v) for v in data.get('vertices', [])]
    indices = data.get('indices')
    return Mesh(vertices, indices)


def save_sagemesh(mesh: Mesh, path: str) -> None:
    os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
    data = {
        'vertices': [list(v) for v in mesh.vertices],
        'indices': mesh.indices,
    }
    with open(path, 'w', encoding='utf-8') as fh:
        json.dump(data, fh, indent=2)
