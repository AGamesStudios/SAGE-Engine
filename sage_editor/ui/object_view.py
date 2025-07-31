import tkinter as tk

from ..style import theme


def build(parent: tk.Widget) -> tk.Frame:
    """Panel showing selected object properties."""
    frame = tk.Frame(parent, bg=theme.PANEL_BG, bd=0, padx=4, pady=4)
    tk.Label(
        frame,
        text="Object View",
        bg=theme.PANEL_BG,
        fg=theme.TEXT,
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Text(
        frame,
        height=10,
        bg=theme.BACKGROUND,
        fg=theme.TEXT,
        relief=tk.FLAT,
        highlightthickness=0,
        bd=0,
        font=("Segoe UI", 10),
    ).pack(expand=True, fill="both")
    return frame
