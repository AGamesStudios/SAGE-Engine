"""Effect registry for SAGE Effects."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Dict, Any, Mapping, Protocol, Iterable, Tuple, Optional


@dataclass
class Frame:
    buffer: bytearray
    width: int
    height: int

    @property
    def stride(self) -> int:
        return self.width * 4


class IEffectsBackend(Protocol):
    def apply_into(self, name: str, src: Frame, dst: Frame, **params: Any) -> None:
        ...

    def pipeline(self, steps: Iterable[Tuple[str, Mapping[str, Any]]], fb: Frame) -> Frame:
        ...

EffectFunc = Callable[..., None]

_registry: Dict[str, EffectFunc] = {}
_param_specs: Dict[str, Dict[str, tuple[type, Any, Any]]] = {
    "blur": {"radius": (int, 0, 100)},
    "glow": {"radius": (int, 0, 100), "color": (tuple, None, None)},
    "pixelate": {"size": (int, 1, 64)},
    "fade": {"alpha": (float, 0.0, 1.0)},
}

class _CPUBackend:
    def __init__(self) -> None:
        self.scissor: Optional[tuple[int, int, int, int]] = None
        self.mask: Optional[Frame] = None
        self.stats: Dict[str, float] = {}

    def apply_into(self, name: str, src: Frame, dst: Frame, **params: Any) -> None:
        dst.buffer[:] = src.buffer
        effect = _registry.get(name)
        if effect:
            if self.scissor:
                x, y, w, h = self.scissor
                stride = dst.stride
                sub = bytearray(w * h * 4)
                for yy in range(h):
                    off = (y + yy) * stride + x * 4
                    sub[yy*w*4:(yy+1)*w*4] = dst.buffer[off:off+w*4]
                effect(sub, w, h, **params)
                for yy in range(h):
                    off = (y + yy) * stride + x * 4
                    dst.buffer[off:off+w*4] = sub[yy*w*4:(yy+1)*w*4]
            else:
                effect(dst.buffer, dst.width, dst.height, **params)
            if self.mask:
                m = self.mask
                for yy in range(min(dst.height, m.height)):
                    for xx in range(min(dst.width, m.width)):
                        moff = yy*m.stride + xx*4
                        alpha = m.buffer[moff + 3]/255
                        off = yy*dst.stride + xx*4
                        for i in range(4):
                            dst.buffer[off+i] = int(dst.buffer[off+i]*alpha)

    def pipeline(self, steps: Iterable[Tuple[str, Mapping[str, Any]]], fb: Frame) -> Frame:
        import time
        a = Frame(bytearray(fb.buffer), fb.width, fb.height)
        b = Frame(bytearray(len(fb.buffer)), fb.width, fb.height)
        src, dst = a, b
        self.stats.clear()
        for i, (name, params) in enumerate(steps):
            start = time.perf_counter()
            if i % 2 == 0:
                src, dst = src, dst
            else:
                src, dst = dst, src
            self.apply_into(name, src, dst, **params)
            self.stats[name] = time.perf_counter() - start
        return dst if steps else fb

_backend_impl: IEffectsBackend = _CPUBackend()
_backend_name: str = "cpu"


def set_backend(name: str) -> None:
    global _backend_name, _backend_impl
    _backend_name = name
    _backend_impl = _CPUBackend()


def get_backend() -> str:
    return _backend_name


def set_scissor(x: int, y: int, w: int, h: int) -> None:
    _backend_impl.scissor = (x, y, w, h)


def clear_scissor() -> None:
    _backend_impl.scissor = None


def set_mask(frame: Frame) -> None:
    _backend_impl.mask = frame


def clear_mask() -> None:
    _backend_impl.mask = None


def register(name: str, func: EffectFunc) -> None:
    """Register a new effect function."""
    _registry[name] = func


def validate(name: str, params: Mapping[str, Any]) -> None:
    spec = _param_specs.get(name)
    if not spec:
        return
    for key, value in params.items():
        if key not in spec:
            raise ValueError(f"Unknown parameter {key} for effect {name}")
        typ, min_v, max_v = spec[key]
        if not isinstance(value, typ):
            raise ValueError(f"{key} must be {typ.__name__}")
        if min_v is not None and value < min_v:
            raise ValueError(f"{key} < {min_v}")
        if max_v is not None and value > max_v:
            raise ValueError(f"{key} > {max_v}")


def apply(name: str, buffer: bytearray, width: int, height: int, **params: Any) -> None:
    validate(name, params)
    fb = Frame(buffer, width, height)
    dst = Frame(buffer, width, height)
    _backend_impl.apply_into(name, fb, dst, **params)


def apply_pipeline(pipeline: Iterable[Tuple[str, Mapping[str, Any]]], buffer: bytearray, width: int, height: int) -> None:
    for name, params in pipeline:
        validate(name, params)
    fb = Frame(buffer, width, height)
    result = _backend_impl.pipeline(pipeline, fb)
    buffer[:] = result.buffer


def list_effects() -> list[str]:
    return list(_registry.keys())


def stats() -> Dict[str, float]:
    return dict(_backend_impl.stats)


def save_preset(path: str, spec: Iterable[Tuple[str, Mapping[str, Any]]]) -> None:
    import json
    with open(path, "w", encoding="utf-8") as f:
        json.dump(list(spec), f)


def load_preset(path: str) -> list[tuple[str, Dict[str, Any]]]:
    import json
    with open(path, "r", encoding="utf-8") as f:
        spec = json.load(f)
    return [(name, params or {}) for name, params in spec]

