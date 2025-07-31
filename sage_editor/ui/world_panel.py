import tkinter as tk

from ..style import theme
from ..core import api_bridge as engine_api
from ..core import state
from .context_menu import ContextMenu


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

    def refresh() -> None:
        box.delete(0, tk.END)
        for obj in engine_api.get_objects():
            name = obj.get("name") or f"obj_{obj['id']}"
            box.insert(tk.END, f"{obj['id']}: {name}")

    def _select(event: tk.Event) -> None:
        if not box.curselection():
            return
        idx = int(box.get(box.curselection()[0]).split(":", 1)[0])
        state.selected_object = idx
        if hasattr(frame, "on_select"):
            frame.on_select(idx)

    box.bind("<<ListboxSelect>>", _select)

    menu = ContextMenu(box)

    def _create() -> None:
        obj_id = engine_api.create_object()
        state.selected_object = obj_id
        refresh()

    def _delete() -> None:
        if not box.curselection():
            return
        idx = int(box.get(box.curselection()[0]).split(":", 1)[0])
        engine_api.delete_object(idx)
        refresh()

    menu.add_command(label="Add Empty Object", command=_create)
    menu.add_command(label="Delete Selected", command=_delete)

    def _show_menu(event: tk.Event) -> None:
        menu.tk_popup(event.x_root, event.y_root)

    box.bind("<Button-3>", _show_menu)

    frame.refresh = refresh

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
