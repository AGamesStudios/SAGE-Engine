import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Central world view area."""

    frame = tk.Frame(parent, bg="#1e1e1e")
    canvas = tk.Canvas(frame, bg="#1e1e1e", highlightthickness=0)
    canvas.pack(expand=True, fill="both")
    canvas.create_rectangle(100, 100, 200, 200, fill="orange")
    frame.canvas = canvas
    return frame
