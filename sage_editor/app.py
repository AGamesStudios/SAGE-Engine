from __future__ import annotations

import sys
import argparse

from PyQt6.QtWidgets import QApplication

from engine.core.scenes.scene import Scene
from engine.entities.game_object import GameObject

from .gui import EditorWindow
from .style import apply_modern_theme


def main(argv: list[str] | None = None) -> int:
    """Launch the SAGE Editor."""
    args = argv or sys.argv[1:]
    parser = argparse.ArgumentParser(description="Launch SAGE Editor")
    parser.add_argument("file", nargs="?", help="Scene or project file")
    ns, qt_args = parser.parse_known_args(args)

    app = QApplication([sys.argv[0], *qt_args])
    apply_modern_theme(app)

    if ns.file:
        from engine.api import load_project, load_scene
        try:
            if ns.file.endswith(".sageproject"):
                proj = load_project(ns.file)
                scene = Scene.from_dict(proj.scene) if proj.scene else Scene()
            else:
                scene = load_scene(ns.file)
        except Exception as exc:  # pragma: no cover - editor startup
            print(f"Cannot load {ns.file}: {exc}")
            scene = Scene()
    else:
        scene = Scene()
        scene.add_object(GameObject(name="origin"))

    window = EditorWindow(scene)
    window.resize(800, 600)
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
