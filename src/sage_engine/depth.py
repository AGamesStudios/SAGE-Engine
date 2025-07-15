"""Layer parallax configuration."""

_parallax: dict[int, float] = {}


def set_parallax(layer: int, factor: float) -> None:
    _parallax[layer] = factor


def get_parallax(layer: int) -> float:
    return _parallax.get(layer, 1.0)
