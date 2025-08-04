"""Entry point for SAGE Engine."""

from sage_engine.logger import setup_logging
from sage_engine.core import boot_engine

# Import modules so they register with the core registry
from sage_engine import window, render, graphic  # noqa: F401

if __name__ == "__main__":
    setup_logging(level="info")
    boot_engine()
