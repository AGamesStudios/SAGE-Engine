"""Qt Hello Sprite demo."""
from PIL import Image, ImageDraw, ImageQt

from sage_engine.gui import GuiApp, QtW

def main() -> None:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(0, 255, 0, 255))

    if QtW is None:
        print("Qt not available, running headless")
        return

    app = GuiApp(None)
    label = QtW.QLabel()
    label.setPixmap(QtW.QPixmap.fromImage(ImageQt.ImageQt(img)))
    app.window.setLayout(QtW.QVBoxLayout())
    app.window.layout().addWidget(label)
    app.run()


if __name__ == "__main__":
    main()
