import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Bottom-right resource manager."""
    frame = tk.Frame(parent, bg="#2c2c2c", padx=4, pady=4)
    tk.Label(
        frame,
        text="Resource Manager",
        bg="#2c2c2c",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Listbox(frame, bg="#1e1e1e", fg="white", relief=tk.FLAT, font=("Segoe UI", 10)).pack(expand=True, fill="both")
    return frame
