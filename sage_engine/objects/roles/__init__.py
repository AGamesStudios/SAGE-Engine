"""Role registry for SAGE Objects."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Dict, Mapping

_role_defs: Dict[str, Mapping[str, object]] = {}


def load_roles(directory: Path | None = None) -> None:
    if directory is None:
        directory = Path(__file__).resolve().parent
    for path in directory.glob("*.role.json"):
        with open(path, "r", encoding="utf8") as fh:
            data = json.load(fh)
        _role_defs[data["id"]] = data


def get_role(role_id: str) -> Mapping[str, object]:
    return _role_defs[role_id]


def registered_roles() -> Mapping[str, Mapping[str, object]]:
    return _role_defs

load_roles()
