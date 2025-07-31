import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    frame = tk.Frame(parent, height=150, bg="#2c2c2c")
    tk.Label(frame, text="Blueprint Designer", bg="#2c2c2c", fg="white").pack(side="left", padx=4, pady=4)
    tk.Label(frame, text="Resource Manager", bg="#2c2c2c", fg="white").pack(side="right", padx=4, pady=4)
    return frame
