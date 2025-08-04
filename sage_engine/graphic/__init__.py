"""Minimal graphic module for SAGE Engine."""

from __future__ import annotations

import logging

from sage_engine.core import register, get
from . import api, color, fx, style  # noqa: F401

log = logging.getLogger("graphic")


def draw_welcome() -> None:
    """Draw background and welcome text using the render system."""
    window = get("window")
    api.draw_rect(0, 0, window.width, window.height, color.BLACK)
    api.draw_text(20, 20, "Welcome to SAGE!", color.WHITE)
    log.info("Drawing welcome message.")


register("draw", draw_welcome)
