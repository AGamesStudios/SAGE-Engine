"""Minimal Hello Sprite example."""
from PIL import Image, ImageDraw, ImageTk

from sage_engine import render


def main() -> None:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    render.Render()
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(255, 0, 0, 255))
    try:
        from tkinter import Tk, Label
        from PIL import ImageTk
        root = Tk()
        root.title("Hello Sprite")
        root.bind("<Escape>", lambda e: root.destroy())
        photo = ImageTk.PhotoImage(img)
        Label(root, image=photo).pack()
        root.mainloop()
    except Exception:
        img.save("hello_sprite.png")
        print("Saved sprite to hello_sprite.png")

if __name__ == "__main__":
    main()
