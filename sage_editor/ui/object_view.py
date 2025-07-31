import tkinter as tk

from ..style import theme
from ..core import api_bridge as engine_api
from ..core import state


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
    name_var = tk.StringVar()
    x_var = tk.StringVar()
    y_var = tk.StringVar()

    def load(obj_id: int) -> None:
        data = engine_api.get_object(obj_id)
        name_var.set(data.get("name", ""))
        x_var.set(str(data.get("x", 0)))
        y_var.set(str(data.get("y", 0)))
        frame.current_object = obj_id

    form = tk.Frame(frame, bg=theme.PANEL_BG)
    form.pack(expand=True, fill="both")
    tk.Label(form, text="Name", bg=theme.PANEL_BG, fg=theme.TEXT).grid(row=0, column=0, sticky="w")
    tk.Entry(form, textvariable=name_var).grid(row=0, column=1, sticky="ew")
    tk.Label(form, text="X", bg=theme.PANEL_BG, fg=theme.TEXT).grid(row=1, column=0, sticky="w")
    tk.Entry(form, textvariable=x_var).grid(row=1, column=1, sticky="ew")
    tk.Label(form, text="Y", bg=theme.PANEL_BG, fg=theme.TEXT).grid(row=2, column=0, sticky="w")
    tk.Entry(form, textvariable=y_var).grid(row=2, column=1, sticky="ew")

    form.columnconfigure(1, weight=1)

    def apply() -> None:
        obj = getattr(frame, "current_object", None)
        if obj is None:
            return
        engine_api.set_object_param(obj, "transform", "x", float(x_var.get()))
        engine_api.set_object_param(obj, "transform", "y", float(y_var.get()))

    def delete() -> None:
        obj = getattr(frame, "current_object", None)
        if obj is None:
            return
        engine_api.delete_object(obj)
        frame.current_object = None
        state.selected_object = None

    btn_frame = tk.Frame(frame, bg=theme.PANEL_BG)
    btn_frame.pack(anchor="e", pady=(4, 0))
    tk.Button(btn_frame, text="Apply Changes", command=apply).pack(side="left", padx=2)
    tk.Button(btn_frame, text="Delete Object", command=delete).pack(side="left", padx=2)

    frame.load_object = load
    frame.current_object = None

    return frame
