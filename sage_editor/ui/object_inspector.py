"""Inspector panel for selected objects."""

import customtkinter as ctk


class ObjectInspector(ctk.CTkFrame):
    """Allows editing object parameters."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Object Inspector")
        self.label.pack(fill="x")
