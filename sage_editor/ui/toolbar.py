"""Top toolbar with quick actions."""

import customtkinter as ctk


class Toolbar(ctk.CTkFrame):
    """Shows quick access buttons."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.run_btn = ctk.CTkButton(self, text="Run Preview")
        self.run_btn.pack(side="left")

