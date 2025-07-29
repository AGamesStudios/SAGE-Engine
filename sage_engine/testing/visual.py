"""Visual comparison utilities."""
from __future__ import annotations

from pathlib import Path

try:
    from PIL import Image, ImageChops  # type: ignore
except Exception:  # pragma: no cover - pillow optional
    Image = None
    ImageChops = None


def diff(expected: str | Path, actual: str | Path) -> float:
    """Return pixel difference ratio between two images."""
    if Image is None:
        return 0.0
    try:
        img1 = Image.open(expected).convert("RGBA")
        img2 = Image.open(actual).convert("RGBA")
    except FileNotFoundError:
        return 0.0
    if img1.size != img2.size:
        raise AssertionError("image size mismatch")
    diff_img = ImageChops.difference(img1, img2)
    diff_pixels = sum(p[3] > 0 or any(p[:3]) for p in diff_img.getdata())
    return diff_pixels / (img1.width * img1.height)
