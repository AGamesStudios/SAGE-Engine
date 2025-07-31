import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    frame = tk.Frame(parent, width=200, bg="#1e1e1e")
    tk.Label(frame, text="World Manager", bg="#1e1e1e", fg="white").pack(anchor="nw", padx=4, pady=4)
    tk.Listbox(frame).pack(expand=True, fill="both", padx=4, pady=4)
    return frame
