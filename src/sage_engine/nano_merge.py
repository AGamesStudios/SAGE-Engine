try:
    import numpy as np  # type: ignore
except Exception:  # pragma: no cover - optional dependency
    np = None
from typing import Iterable, Tuple

__all__ = ["merge_ops"]


def merge_ops(ops: Iterable[Tuple[int, bytes]], simd: bool = True) -> bytes:
    """Sort operations by CRC and scatter the data in one pass."""
    ops_list = list(ops)
    if not ops_list:
        return b""

    if simd and np is not None:
        try:
            crcs = np.fromiter((o[0] for o in ops_list), dtype=np.uint32)
            order = crcs.argsort(kind="stable")
            return b"".join(ops_list[i][1] for i in order)
        except Exception:
            pass

    ops_list.sort(key=lambda o: o[0])
    return b"".join(data for _, data in ops_list)
