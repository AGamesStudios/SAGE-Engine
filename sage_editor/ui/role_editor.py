import tkinter as tk


def build(parent: tk.Widget) -> tk.Frame:
    """Panel to edit roles of the selected object."""
    frame = tk.Frame(parent, bg="#1e1e1e", padx=4, pady=4)
    tk.Label(
        frame,
        text="Role Editor",
        bg="#1e1e1e",
        fg="white",
        font=("Segoe UI", 10, "bold"),
    ).pack(anchor="nw", pady=(0, 4))
    tk.Listbox(frame, bg="#2a2a2a", fg="white", relief=tk.FLAT, font=("Segoe UI", 10)).pack(expand=True, fill="both")
    return frame
