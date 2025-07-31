import tkinter as tk

from ..style import theme


def build(parent: tk.Widget) -> tk.Frame:
    """Panel to edit roles of the selected object."""
    frame = tk.Frame(parent, bg=theme.PANEL_BG, bd=0, padx=4, pady=4)
    tk.Label(
        frame,
        text="Role Editor",
        bg=theme.PANEL_BG,
        fg=theme.TEXT,
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Listbox(
        frame,
        bg=theme.BACKGROUND,
        fg=theme.TEXT,
        relief=tk.FLAT,
        highlightthickness=0,
        bd=0,
        font=("Segoe UI", 10),
    ).pack(expand=True, fill="both")
    return frame
