"""Designer panel for editing blueprint graphs."""

import customtkinter as ctk


class BlueprintDesigner(ctk.CTkFrame):
    """Displays blueprint hierarchy and canvas."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Blueprint Designer")
        self.label.pack(fill="x")

