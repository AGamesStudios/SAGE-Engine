import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Bottom-left blueprint editor area."""
    frame = tk.Frame(parent, bg="#2c2c2c", padx=4, pady=4)
    tk.Label(
        frame,
        text="Blueprint Designer",
        bg="#2c2c2c",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Canvas(frame, bg="#1e1e1e", highlightthickness=0, height=80).pack(expand=True, fill="both")
    return frame
