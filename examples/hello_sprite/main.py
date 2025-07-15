"""Minimal Hello Sprite example."""
from PIL import Image, ImageDraw
import argparse

from sage_engine import render


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--headless", action="store_true")
    args = parser.parse_args()

    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    render._Stub()  # placeholder use
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(255, 0, 0, 255))
    if not args.headless:
        try:
            from tkinter import Tk, Label
            from PIL import ImageTk

            root = Tk()
            root.title("Hello Sprite")
            root.bind("<Escape>", lambda e: root.destroy())
            photo = ImageTk.PhotoImage(img)
            Label(root, image=photo).pack()
            root.mainloop()
            return
        except Exception:
            pass
    img.save("hello_sprite.png")
    print("Saved sprite to hello_sprite.png")

if __name__ == "__main__":
    main()
