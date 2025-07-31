import tkinter as tk

from ..style import theme
from ..core import api_bridge as engine_api
from ..core import state
from .context_menu import ContextMenu


def build(parent: tk.Widget) -> tk.Frame:
    """Central world view area."""

    frame = tk.Frame(parent, bg=theme.BACKGROUND, bd=0)
    canvas = tk.Canvas(frame, bg=theme.BACKGROUND, highlightthickness=0, bd=0)
    canvas.pack(expand=True, fill="both")

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

    def _select(obj_id: int) -> None:
        state.selected_object = obj_id
        if hasattr(frame, "on_select"):
            frame.on_select(obj_id)

    menu = ContextMenu(canvas)

    def _create_object() -> None:
        obj_id = engine_api.create_object()
        _select(obj_id)

    menu.add_command(label="Create Object", command=_create_object)
    menu.add_separator()
    menu.add_command(label="Run Preview", command=engine_api.run_preview)

    def _show_menu(event: tk.Event) -> None:
        menu.tk_popup(event.x_root, event.y_root)

    canvas.bind("<Button-3>", _show_menu)

    def refresh() -> None:
        """Redraw using object data from the engine."""
        canvas.delete("all")
        for obj in engine_api.get_objects():
            x = int(obj.get("x", 0))
            y = int(obj.get("y", 0))
            w = int(obj.get("width", 20))
            h = int(obj.get("height", 20))
            oid = obj["id"]
            fill = theme.ACCENT if state.selected_object == oid else "#444444"
            item = canvas.create_rectangle(x, y, x + w, y + h, fill=fill, outline="")
            canvas.tag_bind(item, "<Button-1>", lambda e, i=oid: _select(i))
        canvas.after(33, refresh)

    frame.canvas = canvas
    frame.refresh = refresh
    return frame
