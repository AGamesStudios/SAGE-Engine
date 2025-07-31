import tkinter as tk

from ..style import theme
from .top_bar import build as build_top_bar
from .main_menu import build as build_main_menu
from .world_panel import build as build_world_panel
from .world_view import build as build_world_view
from .object_view import build as build_object_view
from .role_editor import build as build_role_editor
from .blueprint_designer import build as build_blueprint_designer
from .resource_manager import build as build_resource_manager


def build(parent: tk.Tk) -> tk.Frame:
    """Assemble the main SAGE Studio window."""
    root = tk.Frame(parent, bg=theme.BACKGROUND, bd=0)
    root.rowconfigure(1, weight=1)
    root.columnconfigure(0, weight=1)

    def _run() -> None:
        print("Running preview...")

    def _save() -> None:
        print("Saving project...")

    def _settings() -> None:
        win = tk.Toplevel(root)
        win.title("Settings")
        tk.Label(win, text="Настройки проекта").pack(padx=20, pady=20)

    top = build_top_bar(root, run_cb=_run, save_cb=_save, settings_cb=_settings)
    top.grid(row=0, column=0, sticky="ew")

    panels: dict[str, tk.Frame] = {}

    main = tk.Frame(root, bg=theme.BACKGROUND, bd=0)
    main.grid(row=1, column=0, sticky="nsew")
    main.columnconfigure(0, weight=0)
    main.columnconfigure(1, weight=1)
    main.columnconfigure(2, weight=0)
    main.rowconfigure(0, weight=1)

    world = build_world_panel(main)
    world.grid(row=0, column=0, sticky="ns", padx=(0, 2), pady=2)
    panels["World Manager"] = world

    view = build_world_view(main)
    view.grid(row=0, column=1, sticky="nsew", padx=2, pady=2)

    right = tk.Frame(main, bg=theme.PANEL_BG, bd=0)
    right.grid(row=0, column=2, sticky="ns", padx=(2, 0), pady=2)
    right.rowconfigure(0, weight=1)
    right.rowconfigure(1, weight=1)
    right.columnconfigure(0, weight=1)

    obj = build_object_view(right)
    obj.grid(row=0, column=0, sticky="nsew")
    panels["Object View"] = obj

    role = build_role_editor(right)
    role.grid(row=1, column=0, sticky="nsew", pady=(4, 0))
    panels["Role Editor"] = role

    bottom = tk.Frame(root, bg=theme.BACKGROUND, bd=0)
    bottom.grid(row=2, column=0, sticky="ew")
    bottom.columnconfigure(0, weight=1)
    bottom.columnconfigure(1, weight=1)

    blue = build_blueprint_designer(bottom)
    blue.grid(row=0, column=0, sticky="nsew", padx=(0, 2))
    panels["Blueprint Designer"] = blue

    res = build_resource_manager(bottom)
    res.grid(row=0, column=1, sticky="nsew", padx=(2, 0))
    panels["Resource Manager"] = res

    def _toggle_panel(name: str, visible: bool) -> None:
        frame = panels[name]
        if visible:
            frame.grid()
        else:
            frame.grid_remove()

    build_main_menu(parent, {n: lambda v, n=n: _toggle_panel(n, v) for n in panels})

    return root
