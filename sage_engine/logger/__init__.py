"""Simple logging setup for SAGE Engine."""

from __future__ import annotations

import logging


def setup_logging(level: str = "info") -> None:
    """Configure root logger with unified format."""
    lvl = getattr(logging, level.upper(), logging.INFO)
    logging.basicConfig(level=lvl, format="[%(levelname)s] [%(name)s] %(message)s")


def get_logger(name: str) -> logging.Logger:
    """Return logger with ``name``."""
    return logging.getLogger(name)
