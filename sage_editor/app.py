from __future__ import annotations

import sys

from PyQt6.QtWidgets import QApplication, QStyleFactory

from engine.core.scenes.scene import Scene
from engine.entities.game_object import GameObject

from .gui import EditorWindow


def main(argv: list[str] | None = None) -> int:
    """Launch the SAGE Editor."""
    app = QApplication(argv or sys.argv)
    app.setStyle(QStyleFactory.create("Fusion"))

    # minimal scene with a single object
    scene = Scene()
    scene.add_object(GameObject(name="origin"))

    window = EditorWindow(scene)
    window.resize(800, 600)
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
