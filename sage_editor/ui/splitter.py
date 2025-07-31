import tkinter as tk

class Splitter(tk.PanedWindow):
    """Light wrapper around :class:`tk.PanedWindow` used across the editor."""

    def __init__(self, master: tk.Widget, orient: str = tk.HORIZONTAL, **kw) -> None:
        super().__init__(master, orient=orient, sashrelief=tk.FLAT, bg='#2b2b2b', showhandle=False, **kw)

