import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Bottom bar with blueprint and resource sections."""

    frame = tk.Frame(parent, height=150, bg="#2c2c2c", padx=4, pady=4)
    tk.Label(
        frame,
        text="Blueprint Designer",
        bg="#2c2c2c",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(side="left", padx=4)

    tk.Label(frame, text="Resource Manager", bg="#2c2c2c", fg="white", font=("Segoe UI", 10)).pack(side="right", padx=4)
    return frame
