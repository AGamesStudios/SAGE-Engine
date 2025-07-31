import tkinter as tk

from .top_bar import build as build_top_bar
from .world_panel import build as build_world_panel
from .world_view import build as build_world_view
from .object_view import build as build_object_view
from .role_editor import build as build_role_editor
from .blueprint_designer import build as build_blueprint_designer
from .resource_manager import build as build_resource_manager


def build(parent: tk.Widget) -> tk.Frame:
    """Assemble the main SAGE Studio window."""
    root = tk.Frame(parent, bg="#1f1f1f")
    root.rowconfigure(1, weight=1)
    root.columnconfigure(0, weight=1)

    top = build_top_bar(root)
    top.grid(row=0, column=0, sticky="ew")

    main = tk.Frame(root, bg="#1f1f1f")
    main.grid(row=1, column=0, sticky="nsew")
    main.columnconfigure(0, weight=0)
    main.columnconfigure(1, weight=1)
    main.columnconfigure(2, weight=0)
    main.rowconfigure(0, weight=1)

    world = build_world_panel(main)
    world.grid(row=0, column=0, sticky="ns", padx=(0, 2), pady=2)

    view = build_world_view(main)
    view.grid(row=0, column=1, sticky="nsew", padx=2, pady=2)

    right = tk.Frame(main, bg="#1e1e1e")
    right.grid(row=0, column=2, sticky="ns", padx=(2, 0), pady=2)
    right.rowconfigure(0, weight=1)
    right.rowconfigure(1, weight=1)
    right.columnconfigure(0, weight=1)

    obj = build_object_view(right)
    obj.grid(row=0, column=0, sticky="nsew")

    role = build_role_editor(right)
    role.grid(row=1, column=0, sticky="nsew", pady=(4, 0))

    bottom = tk.Frame(root, bg="#1f1f1f")
    bottom.grid(row=2, column=0, sticky="ew")
    bottom.columnconfigure(0, weight=1)
    bottom.columnconfigure(1, weight=1)

    blue = build_blueprint_designer(bottom)
    blue.grid(row=0, column=0, sticky="nsew", padx=(0, 2))

    res = build_resource_manager(bottom)
    res.grid(row=0, column=1, sticky="nsew", padx=(2, 0))

    return root
