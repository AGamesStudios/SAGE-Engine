"""Editor for FlowScript logic."""

import customtkinter as ctk


class FlowScriptEditor(ctk.CTkFrame):
    """Write and test FlowScript snippets."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.text = ctk.CTkTextbox(self, height=150)
        self.text.pack(fill="both", expand=True)

