import tkinter as tk

from ..style import theme


def build(parent: tk.Widget) -> tk.Frame:
    """Bottom-left blueprint editor area."""
    frame = tk.Frame(parent, bg=theme.PANEL_BG, bd=0, padx=4, pady=4)
    tk.Label(
        frame,
        text="Blueprint Designer",
        bg=theme.PANEL_BG,
        fg=theme.TEXT,
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Canvas(
        frame,
        bg=theme.BACKGROUND,
        highlightthickness=0,
        bd=0,
        height=80,
    ).pack(expand=True, fill="both")
    return frame
