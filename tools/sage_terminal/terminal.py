"""Graphical SAGE Terminal based on CustomTkinter."""
from __future__ import annotations

try:
    import customtkinter as ctk
except Exception as exc:  # pragma: no cover - import check
    raise SystemExit(f"customtkinter missing: {exc}")

from tools.sage_terminal import commands
from .utils import check_dependencies


class TerminalApp:
    def __init__(self) -> None:
        check_dependencies()
        ctk.set_default_color_theme("blue")
        self.root = ctk.CTk()
        self.root.title("SAGE Terminal v1.0")
        self.output = ctk.CTkTextbox(self.root, height=300, width=600)
        self.output.pack(fill="both", expand=True, padx=5, pady=5)
        self.entry = ctk.CTkEntry(self.root)
        self.entry.pack(fill="x", padx=5, pady=5)
        self.entry.bind("<Return>", self.on_enter)
        self.entry.bind("<Up>", self.on_prev)
        self.entry.bind("<Down>", self.on_next)
        self.history: list[str] = []
        self.hindex = 0
        self.write_line("SAGE Terminal v1.0 - type 'help'")

    def write_line(self, text: str) -> None:
        self.output.insert("end", text + "\n")
        self.output.see("end")

    def on_enter(self, _event=None) -> None:  # type: ignore[override]
        line = self.entry.get().strip()
        if not line:
            return
        self.entry.delete(0, "end")
        self.history.append(line)
        self.hindex = len(self.history)
        self.write_line(f"> {line}")
        result = commands.execute(self, line)
        if result:
            self.write_line(result)

    def on_prev(self, _event=None) -> None:
        if self.history and self.hindex > 0:
            self.hindex -= 1
            self.entry.delete(0, "end")
            self.entry.insert(0, self.history[self.hindex])

    def on_next(self, _event=None) -> None:
        if self.history and self.hindex < len(self.history) - 1:
            self.hindex += 1
            self.entry.delete(0, "end")
            self.entry.insert(0, self.history[self.hindex])
        else:
            self.entry.delete(0, "end")
            self.hindex = len(self.history)

    def close(self) -> None:
        self.root.quit()

    def run(self) -> None:
        self.root.mainloop()


def main() -> None:
    try:
        TerminalApp().run()
    except Exception as exc:  # pragma: no cover - runtime guard
        try:
            import tkinter.messagebox as messagebox
            messagebox.showerror("SAGE Terminal", str(exc))
        except Exception:
            print("Error:", exc)


if __name__ == "__main__":  # pragma: no cover - manual launch
    main()
