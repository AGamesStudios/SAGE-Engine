"""Qt Hello Sprite demo using the pluggable backend."""
from pathlib import Path

from PIL import Image, ImageDraw, ImageQt

from sage_engine import gui
from sage_engine.ui import theme


def main() -> None:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(0, 255, 0, 255))

    backend = gui.load_backend("qt6")
    if backend.__class__.__name__ == "HeadlessBackend":
        print("Qt not available, running headless")
        return

    backend.create_window(200, 200, "SAGE Engine")
    from PyQt6 import QtWidgets as QtW, QtGui as QtG

    label = QtW.QLabel()
    label.setPixmap(QtG.QPixmap.fromImage(ImageQt.ImageQt(img)))
    layout = QtW.QVBoxLayout()
    layout.addWidget(label)
    menu_bar = QtW.QMenuBar()
    menu = menu_bar.addMenu("Theme")
    dark = QtW.QAction("Dark", menu)
    light = QtW.QAction("Light", menu)

    def switch(name: str) -> None:
        theme.set_theme(str(Path(__file__).with_name(f"{name}.vel")))

    dark.triggered.connect(lambda: switch("dark"))
    light.triggered.connect(lambda: switch("light"))
    menu.addAction(dark)
    menu.addAction(light)
    backend._window.setMenuBar(menu_bar)  # type: ignore[attr-defined]
    backend._window.setLayout(layout)  # type: ignore[attr-defined]
    backend._app.exec()  # type: ignore[attr-defined]


if __name__ == "__main__":
    main()
