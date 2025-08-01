from __future__ import annotations

"""Helpers for loading role metadata with UI descriptors."""

from pathlib import Path
import json
from typing import Any, Dict


def load_role_metadata(role_name: str) -> Dict[str, Any]:
    path = Path(__file__).resolve().parents[2] / "roles" / f"{role_name.capitalize()}.role.json"
    if not path.exists():
        return {}
    with open(path, "r", encoding="utf8") as fh:
        return json.load(fh)
