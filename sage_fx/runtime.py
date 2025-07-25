"""Runtime for applying Feather-FX effects."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
from typing import Dict, List


from .optimizer import optimize_ops
from .glsl_backend import generate_glsl
# FIXME retro_backend to interpret on CPU
from .parser import Operation, parse_fx


_cache: Dict[str, List[Operation]] = {}


def _hash_file(path: Path) -> str:
    data = path.read_bytes()
    return hashlib.sha256(data).hexdigest()


def load_fx(path: str | Path) -> List[Operation]:
    """Load and cache an effect."""
    p = Path(path)
    key = _hash_file(p)
    if key not in _cache:
        ops = parse_fx(p)
        _cache[key] = optimize_ops(ops)
    return _cache[key]


def _detect_backend() -> str:
    return os.environ.get("FEATHER_FX_BACKEND", "cpu")


def apply_fx(surface: object, fx: List[Operation]) -> List[str]:
    """Apply operations, returning a log for testing."""
    backend = _detect_backend()
    log: List[str] = [f"backend={backend}"]
    if backend == "gpu":
        shader = generate_glsl(fx)
        log.append("shader")
        log.append(shader)
    for op in fx:
        log.append(op.name)
    return log
