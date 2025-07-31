import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    frame = tk.Frame(parent, width=250, bg="#1e1e1e")
    tk.Label(frame, text="Object View", bg="#1e1e1e", fg="white").pack(anchor="nw", padx=4, pady=(4,2))
    tk.Label(frame, text="Role Editor", bg="#1e1e1e", fg="white").pack(anchor="nw", padx=4, pady=(2,4))
    return frame
