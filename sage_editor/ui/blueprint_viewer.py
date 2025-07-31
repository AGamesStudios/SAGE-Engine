"""Viewer for blueprint assets."""

import customtkinter as ctk


class BlueprintViewer(ctk.CTkFrame):
    """Displays blueprint hierarchy."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Blueprint Viewer")
        self.label.pack(fill="x")
