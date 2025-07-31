import tkinter as tk

class ContextMenu(tk.Menu):
    """Simple styled context menu."""

    def __init__(self, parent: tk.Widget, **kwargs) -> None:
        defaults = dict(
            tearoff=0,
            bg="#2b2b2b",
            fg="#f0f0f0",
            activebackground="#ffaa00",
            activeforeground="#000000",
            relief=tk.FLAT,
        )
        defaults.update(kwargs)
        super().__init__(parent, **defaults)

