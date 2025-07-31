"""Panel showing properties of the selected object."""

import customtkinter as ctk


class ObjectView(ctk.CTkFrame):
    """Allows editing object parameters."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Object View")
        self.label.pack(fill="x")

