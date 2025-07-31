import tkinter as tk

from ..style import theme
from ..runtime import engine_api


def build(parent: tk.Widget) -> tk.Frame:
    """Central world view area."""

    frame = tk.Frame(parent, bg=theme.BACKGROUND, bd=0)
    canvas = tk.Canvas(frame, bg=theme.BACKGROUND, highlightthickness=0, bd=0)
    canvas.pack(expand=True, fill="both")
    canvas.create_rectangle(100, 100, 200, 200, fill=theme.ACCENT)

    def refresh() -> None:
        """Temporary redraw using engine preview."""
        canvas.delete("all")
        # placeholder: ask engine for preview frame; here we just draw a rect
        engine_api.run_preview()
        canvas.create_rectangle(100, 100, 200, 200, fill=theme.ACCENT)

    frame.canvas = canvas
    frame.refresh = refresh
    return frame
