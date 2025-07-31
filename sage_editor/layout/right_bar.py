import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Build right panel for object view and role editor."""

    frame = tk.Frame(parent, width=250, bg="#1e1e1e", padx=4, pady=4)
    tk.Label(
        frame,
        text="Object View",
        bg="#1e1e1e",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))

    tk.Label(frame, text="Role Editor", bg="#1e1e1e", fg="white", font=("Segoe UI", 10)).pack(anchor="nw")
    return frame
