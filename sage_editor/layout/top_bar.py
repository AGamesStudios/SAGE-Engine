import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    frame = tk.Frame(parent, height=30, bg="#2c2c2c")
    tk.Label(frame, text="SAGE Studio", bg="#2c2c2c", fg="white").pack(side="left", padx=8)

    tk.Button(frame, text="Run Preview").pack(side="left", padx=4)
    tk.Button(frame, text="Save").pack(side="left", padx=4)
    tk.Button(frame, text="Settings").pack(side="left", padx=4)
    return frame
