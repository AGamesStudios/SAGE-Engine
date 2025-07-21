"""Parser for .sage_fx files."""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List


@dataclass
class Operation:
    name: str
    args: Dict[str, Any]


_ALLOWED_OPS = {
    "blit": [],
    "blend_add": ["factor"],
    "blend_mul": ["factor"],
    "lookup_rgb": ["lut"],
    "bloom_lut": ["lut", "strength"],
    "outline": ["color", "radius"],
    "shift_uv": ["x", "y"],
    "mask_alpha": ["threshold"],
    "threshold": ["level"],
    "noop": [],
}


class FXParseError(Exception):
    pass


def parse_fx(path: str | Path) -> List[Operation]:
    """Parse a .sage_fx file and return a list of operations."""
    lines = Path(path).read_text(encoding="utf-8").splitlines()
    ops: List[Operation] = []
    in_pass = False
    for raw in lines:
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.upper().startswith("PASS"):
            in_pass = True
            continue
        if not in_pass:
            continue
        tokens = line.split()
        if not tokens:
            continue
        name = tokens[0]
        if name not in _ALLOWED_OPS:
            raise FXParseError(f"Unknown op '{name}'")
        args: Dict[str, Any] = {}
        for token in tokens[1:]:
            if ":" in token:
                key, value = token.split(":", 1)
            else:
                key = _ALLOWED_OPS[name][0] if _ALLOWED_OPS[name] else "value"
                value = token
            args[key] = value
        missing = [k for k in _ALLOWED_OPS[name] if k not in args]
        if missing:
            raise FXParseError(f"Missing arguments {missing} for op '{name}'")
        ops.append(Operation(name=name, args=args))
    return ops
