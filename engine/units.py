"""Unit conversion helpers for SAGE Engine."""
UNITS_PER_METER = 1.0


def set_units_per_meter(scale: float) -> None:
    """Set how many engine units represent one meter."""
    global UNITS_PER_METER
    if scale <= 0:
        raise ValueError("scale must be positive")
    UNITS_PER_METER = float(scale)


def to_units(meters: float) -> float:
    """Convert meters to engine units."""
    return meters * UNITS_PER_METER


def from_units(value: float) -> float:
    """Convert engine units back to meters."""
    return value / UNITS_PER_METER


def meters(value: float) -> float:
    """Alias for :func:`to_units`."""
    return to_units(value)


def kilometers(value: float) -> float:
    """Convert kilometers to engine units."""
    return to_units(value * 1000.0)
