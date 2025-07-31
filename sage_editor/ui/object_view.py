import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Panel showing selected object properties."""
    frame = tk.Frame(parent, bg="#1e1e1e", padx=4, pady=4)
    tk.Label(
        frame,
        text="Object View",
        bg="#1e1e1e",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Text(frame, height=10, bg="#2a2a2a", fg="white", relief=tk.FLAT, font=("Segoe UI", 10)).pack(expand=True, fill="both")
    return frame
