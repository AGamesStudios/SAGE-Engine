import tkinter as tk

from ..style import theme
from ..core import state
from .top_bar import build as build_top_bar
from .main_menu import build as build_main_menu
from .world_panel import build as build_world_panel
from .world_view import build as build_world_view
from .object_view import build as build_object_view
from .role_editor import build as build_role_editor
from .blueprint_designer import build as build_blueprint_designer
from .resource_manager import build as build_resource_manager
from .splitter import Splitter


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
    parents: dict[str, Splitter] = {}

    main_split = Splitter(root, orient=tk.HORIZONTAL)
    main_split.grid(row=1, column=0, sticky="nsew")

    world = build_world_panel(main_split)
    main_split.add(world)
    panels["World Manager"] = world
    parents["World Manager"] = main_split

    center_split = Splitter(main_split, orient=tk.HORIZONTAL)
    main_split.add(center_split)

    view_split = Splitter(center_split, orient=tk.VERTICAL)
    center_split.add(view_split)

    view = build_world_view(view_split)
    view_split.add(view)

    bottom_split = Splitter(view_split, orient=tk.HORIZONTAL)
    view_split.add(bottom_split)

    blue = build_blueprint_designer(bottom_split)
    bottom_split.add(blue)
    panels["Blueprint Designer"] = blue
    parents["Blueprint Designer"] = bottom_split

    res = build_resource_manager(bottom_split)
    bottom_split.add(res)
    panels["Resource Manager"] = res
    parents["Resource Manager"] = bottom_split

    right_split = Splitter(center_split, orient=tk.VERTICAL)
    center_split.add(right_split)

    obj = build_object_view(right_split)
    right_split.add(obj)
    panels["Object View"] = obj
    parents["Object View"] = right_split

    role = build_role_editor(right_split)
    right_split.add(role)
    panels["Role Editor"] = role
    parents["Role Editor"] = right_split

    def _toggle_panel(name: str, visible: bool) -> None:
        frame = panels[name]
        pw = parents.get(name)
        if not pw:
            return
        if visible:
            if str(frame) not in pw.panes():
                pw.add(frame)
        else:
            if str(frame) in pw.panes():
                pw.forget(frame)
        state.panels[name] = visible

    build_main_menu(parent, {n: lambda v, n=n: _toggle_panel(n, v) for n in panels})

    def _apply_positions() -> None:
        if "main" in state.splitters:
            main_split.sash_place(0, state.splitters["main"], 0)
        if "center" in state.splitters:
            center_split.sash_place(0, state.splitters["center"], 0)
        if "bottom" in state.splitters:
            view_split.sash_place(0, 0, state.splitters["bottom"])
        if "right" in state.splitters:
            right_split.sash_place(0, 0, state.splitters["right"])

    def _get_positions() -> None:
        state.splitters["main"] = main_split.sash_coord(0)[0]
        state.splitters["center"] = center_split.sash_coord(0)[0]
        state.splitters["bottom"] = view_split.sash_coord(0)[1]
        state.splitters["right"] = right_split.sash_coord(0)[1]

    root.after(100, _apply_positions)
    root.get_split_positions = _get_positions

    view.refresh()
    world.refresh()

    return root
