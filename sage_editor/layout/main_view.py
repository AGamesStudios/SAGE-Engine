import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    frame = tk.Frame(parent, bg="#f0f0f0")
    canvas = tk.Canvas(frame, bg="white")
    canvas.pack(expand=True, fill="both")
    return frame
