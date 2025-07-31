import tkinter as tk

from ..style import theme
from ..runtime import engine_api


def build(parent: tk.Widget) -> tk.Frame:
    """Central world view area."""

    frame = tk.Frame(parent, bg=theme.BACKGROUND, bd=0)
    canvas = tk.Canvas(frame, bg=theme.BACKGROUND, highlightthickness=0, bd=0)
    canvas.pack(expand=True, fill="both")
    canvas.create_rectangle(100, 100, 200, 200, fill=theme.ACCENT)

    canvas.scale_factor = 1.0

    def _on_zoom(event: tk.Event) -> None:
        factor = 1.1 if event.delta > 0 else 0.9
        canvas.scale("all", event.x, event.y, factor, factor)
        canvas.scale_factor *= factor

    def _start_pan(event: tk.Event) -> None:
        canvas.scan_mark(event.x, event.y)

    def _pan(event: tk.Event) -> None:
        canvas.scan_dragto(event.x, event.y, gain=1)

    canvas.bind("<MouseWheel>", _on_zoom)
    canvas.bind("<ButtonPress-1>", _start_pan)
    canvas.bind("<B1-Motion>", _pan)

    def refresh() -> None:
        """Temporary redraw using engine preview."""
        canvas.delete("all")
        # placeholder: ask engine for preview frame; here we just draw a rect
        engine_api.run_preview()
        canvas.create_rectangle(100, 100, 200, 200, fill=theme.ACCENT)
        canvas.after(33, refresh)

    frame.canvas = canvas
    frame.refresh = refresh
    return frame
