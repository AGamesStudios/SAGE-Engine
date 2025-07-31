import tkinter as tk

from ..style import theme


def build(parent: tk.Widget) -> tk.Frame:
    """Build left panel with a simple scene list."""

    frame = tk.Frame(parent, width=200, bg=theme.PANEL_BG, bd=0, padx=4, pady=4)
    tk.Label(
        frame,
        text="World Manager",
        bg=theme.PANEL_BG,
        fg=theme.TEXT,
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))

    box = tk.Listbox(
        frame,
        bg=theme.BACKGROUND,
        fg=theme.TEXT,
        relief=tk.FLAT,
        highlightthickness=0,
        bd=0,
        font=("Segoe UI", 10),
    )
    box.pack(expand=True, fill="both")

    def _drag_start(event: tk.Event) -> None:
        box._drag_data = box.get(box.nearest(event.y))

    def _drag_end(event: tk.Event) -> None:
        item = getattr(box, "_drag_data", None)
        if item:
            print(f"Dropped scene: {item}")
            box._drag_data = None

    box.bind("<ButtonPress-1>", _drag_start)
    box.bind("<ButtonRelease-1>", _drag_end)

    return frame
