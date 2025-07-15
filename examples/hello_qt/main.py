"""Qt Hello Sprite demo using the pluggable backend."""
from PIL import Image, ImageDraw, ImageQt

from sage_engine import gui


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
    backend._window.setLayout(layout)  # type: ignore[attr-defined]
    backend.process_events()


if __name__ == "__main__":
    main()
