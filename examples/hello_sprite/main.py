"""Minimal Hello Sprite example."""
from PIL import Image, ImageDraw
import argparse

from sage_engine import gui


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gui", default="auto")
    args = parser.parse_args()

    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(255, 0, 0, 255))
    backend = gui.load_backend(args.gui)
    if not isinstance(backend, gui.headless.HeadlessBackend):
        try:
            backend.create_window(200, 200, "Hello Sprite")
            if backend.__class__.__module__.startswith("gui_qt"):
                from PyQt6 import QtWidgets as QtW, QtGui as QtG
                from PIL import ImageQt

                label = QtW.QLabel()
                label.setPixmap(QtG.QPixmap.fromImage(ImageQt.ImageQt(img)))
                layout = QtW.QVBoxLayout()
                layout.addWidget(label)
                backend._window.setLayout(layout)  # type: ignore[attr-defined]
                backend._app.exec()  # type: ignore[attr-defined]
            else:
                from tkinter import Label
                from PIL import ImageTk

                photo = ImageTk.PhotoImage(img)
                lbl = Label(backend.root, image=photo)  # type: ignore[attr-defined]
                lbl.pack()
                backend.root.mainloop()  # type: ignore[attr-defined]
            return
        except Exception as exc:  # pragma: no cover - UI
            print(f"GUI failed: {exc}")
    img.save("hello_sprite.png")
    print("Saved sprite to hello_sprite.png")

if __name__ == "__main__":
    main()
