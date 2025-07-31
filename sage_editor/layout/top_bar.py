import tkinter as tk


def build(
    parent: tk.Widget,
    run_cb=None,
    save_cb=None,
    settings_cb=None,
) -> tk.Frame:
    """Build the top bar with a few action buttons."""

    frame = tk.Frame(parent, height=30, bg="#2c2c2c", padx=4, pady=2)

    tk.Label(
        frame,
        text="SAGE Studio",
        bg="#2c2c2c",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(side="left", padx=(0, 8))

    style = {
        "bg": "#2a2a2a",
        "fg": "#f4a261",
        "activebackground": "#3a3a3a",
        "relief": tk.FLAT,
        "font": ("Segoe UI", 10),
    }

    tk.Button(frame, text="Run Preview", command=run_cb, **style).pack(side="left", padx=4)
    tk.Button(frame, text="Save", command=save_cb, **style).pack(side="left", padx=4)
    tk.Button(frame, text="Settings", command=settings_cb, **style).pack(side="left", padx=4)

    return frame
