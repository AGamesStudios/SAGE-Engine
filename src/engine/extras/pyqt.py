"""Wrapper for the optional PyQt6 dependency."""

from __future__ import annotations

import importlib


def refresh() -> bool:
    """Refresh PyQt6 availability and modules."""
    global QtCore, QtWidgets, QtGui, QtOpenGLWidgets, AVAILABLE
    try:
        QtCore = importlib.import_module("PyQt6.QtCore")  # type: ignore
    except Exception:  # pragma: no cover - optional dependency
        AVAILABLE = False
        QtCore = None  # type: ignore
        QtWidgets = None  # type: ignore
        QtGui = None  # type: ignore
        QtOpenGLWidgets = None  # type: ignore
        return False
    AVAILABLE = True
    try:
        QtWidgets = importlib.import_module("PyQt6.QtWidgets")  # type: ignore
    except Exception:
        QtWidgets = None  # type: ignore
    try:
        QtGui = importlib.import_module("PyQt6.QtGui")  # type: ignore
    except Exception:
        QtGui = None  # type: ignore
    try:
        QtOpenGLWidgets = importlib.import_module("PyQt6.QtOpenGLWidgets")  # type: ignore
    except Exception:
        QtOpenGLWidgets = None  # type: ignore
    return True


refresh()

__all__ = [
    "AVAILABLE",
    "QtWidgets",
    "QtCore",
    "QtGui",
    "QtOpenGLWidgets",
    "refresh",
]
