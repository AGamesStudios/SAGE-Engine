"""Top application menu for SAGE Studio."""
from __future__ import annotations

import tkinter as tk
from typing import Callable, Mapping


def build(root: tk.Tk, toggles: Mapping[str, Callable[[bool], None]]) -> tk.Menu:
    """Create the main menu and attach to *root*."""
    menubar = tk.Menu(root)

    file_menu = tk.Menu(menubar, tearoff=0)
    file_menu.add_command(label="New Project", command=lambda: print("New Project"))
    file_menu.add_command(label="Open Project", command=lambda: print("Open Project"))
    file_menu.add_command(label="Save Project", command=lambda: print("Save Project"))
    file_menu.add_separator()
    file_menu.add_command(label="Exit", command=root.quit)
    menubar.add_cascade(label="File", menu=file_menu)

    edit_menu = tk.Menu(menubar, tearoff=0)
    edit_menu.add_command(label="Undo", command=lambda: print("Undo"))
    edit_menu.add_command(label="Redo", command=lambda: print("Redo"))
    edit_menu.add_separator()
    edit_menu.add_command(label="Preferences", command=lambda: print("Prefs"))
    menubar.add_cascade(label="Edit", menu=edit_menu)

    view_menu = tk.Menu(menubar, tearoff=0)
    for name, callback in toggles.items():
        var = tk.BooleanVar(value=True)

        def _toggle(n=name, v=var, cb=callback) -> None:
            cb(bool(v.get()))

        view_menu.add_checkbutton(label=name, variable=var, command=_toggle)
    menubar.add_cascade(label="View", menu=view_menu)

    tools_menu = tk.Menu(menubar, tearoff=0)
    tools_menu.add_command(label="Compile Game", command=lambda: print("Compile"))
    tools_menu.add_command(label="Run Preview", command=lambda: print("Run Preview"))
    tools_menu.add_command(label="Export Assets", command=lambda: print("Export"))
    menubar.add_cascade(label="Tools", menu=tools_menu)

    help_menu = tk.Menu(menubar, tearoff=0)
    help_menu.add_command(label="Documentation", command=lambda: print("Docs"))
    help_menu.add_command(label="Keyboard Shortcuts", command=lambda: print("Keys"))
    help_menu.add_command(label="About SAGE Studio", command=lambda: print("About"))
    menubar.add_cascade(label="Help", menu=help_menu)

    root.config(menu=menubar)
    return menubar
