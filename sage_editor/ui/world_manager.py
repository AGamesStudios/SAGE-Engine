"""Panel for switching worlds and scenes."""

import customtkinter as ctk


class WorldManager(ctk.CTkFrame):
    """Allows scene selection and layer management."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="World Manager")
        self.label.pack(fill="x")

